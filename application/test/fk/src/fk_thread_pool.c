/**************************************************************
* 文件名称:  
* 作            者:  
* 日            期:  
* 文件描述:
* 版            本: 
*     <修改者>     <时间>      <版本 >     <描述>                                                           增加任务执行完毕回调
**************************************************************/

#include "fk_thread_pool.h"
#include "fk_common.h"

int
tf_thread_pool_worker_type_set_release_cb (
                THREAD_POOL                 *p_thread_pool,
                const uint8_t               worker_type,
                THREAD_WORKER_RELEASE_CB    release_cb)
{
    if (!p_thread_pool)
    {
        printf("Invalid thread pool.\r\n");
        return -1;
    }

    if(worker_type >= MAX_WORKER_TYPE_NUM)
    {
        printf("set worker type limit failed due to invalid worker type.\r\n");
        return -1;
    }

    p_thread_pool->type_info[worker_type].release_cb = release_cb;

    return 0;
}

int
tf_thread_pool_worker_type_set_limit (
                THREAD_POOL     *p_thread_pool,
                const uint8_t   worker_type,
                const uint16_t  limit)
{
    if (!p_thread_pool)
    {
        printf("Invalid thread pool.\r\n");
        return 1;
    }

    if(worker_type >= MAX_WORKER_TYPE_NUM)
    {
        printf("set worker type limit failed due to invalid worker type.\r\n");
        return -1;
    }

    if(limit > p_thread_pool->max_thread_num)
    {
        printf("set worker type limit failed due to invalid limit number.\r\n");
        return -1;
    }

    p_thread_pool->type_info[worker_type].thread_limit = limit;

    return 0;
}

int
tf_thread_pool_add_worker (
                THREAD_POOL             *p_thread_pool,
                const THREAD_WORKER_CB  handler,
                const uint8_t           worker_type,
                uint32_t                key,
                const uint32_t          arg1)
{
    TF_NODE_WORKER *p_new_worker;

    //无效onu物理ID转换任务链节点ID
    KEY_PHYUINT2KEY(key);

    if(key == INVALID_KEY || key >= MAX_WORKER_NUM)
    {
        printf("Add worker failed due to invalid key.\r\n");
        return -1;
    }

    if(worker_type >= MAX_WORKER_TYPE_NUM)
    {
        printf("Add worker failed due to invalid worker type.\r\n");
        return -1;
    }

    if (!p_thread_pool)
    {
        printf("Invalid thread pool.\r\n");
        return -1;
    }

    p_new_worker = (TF_NODE_WORKER *)malloc(sizeof (TF_NODE_WORKER));
    if(p_new_worker == NULL)
    {
        printf("Malloc worker memory for key %d failed.\r\n", key);
        return -1;
    }

    p_new_worker->handler     = handler;
    p_new_worker->arg1        = arg1;
    p_new_worker->worker_type = worker_type;
    p_new_worker->remove_flag = 0;
  
    pthread_mutex_lock (&p_thread_pool->worker_list_lock);

    STAILQ_INSERT_TAIL(&p_thread_pool->p_node_worker_list[key], p_new_worker, next);
    
    tf_net_bitlist_set(p_thread_pool->worker_bitmap[PHY_UINT_2_PHY_ID(key)], PHY_UINT_2_PHY_ID(key));
    p_thread_pool->type_info[worker_type].worker_cnt += 1;

    pthread_mutex_unlock (&p_thread_pool->worker_list_lock);

    pthread_cond_signal (&p_thread_pool->worker_list_cond);

    return 0;
}

int
tf_thread_pool_clear_worker (
                THREAD_POOL     *p_thread_pool,
                uint32_t        key,
                const uint8_t   worker_type)
{
    TF_NODE_WORKER *p_worker;
    
    //无效onu物理ID转换任务链节点ID
    KEY_PHYUINT2KEY(key);

    if(key == INVALID_KEY || key >= MAX_WORKER_NUM)
    {
        printf("Clear worker failed due to invalid key.\r\n");
        return -1;
    }

    if(worker_type >= MAX_WORKER_TYPE_NUM)
    {
        printf("Clear worker failed due to invalid worker type.\r\n");
        return -1;
    }

    if (!p_thread_pool)
    {
        printf("Invalid thread pool.\r\n");
        return -1;
    }

    pthread_mutex_lock (&p_thread_pool->worker_list_lock);

    STAILQ_FOREACH(p_worker, &p_thread_pool->p_node_worker_list[key], next)
    {
        if(p_worker->worker_type == worker_type)
            p_worker->remove_flag = 1;  
    }

    pthread_mutex_unlock (&p_thread_pool->worker_list_lock);

    return 0;
}

//节点设备是否正在执行任务
static int
tf_thread_pool_node_worker_is_busy(THREAD_POOL *p_thread_pool, uint32_t key)
{
    uint16_t idx;
    
    for(idx = 0; idx < p_thread_pool->max_thread_num; idx++)
    {
        if(key == p_thread_pool->p_processing_key_queue[idx])
            break;
    }

    if(idx < p_thread_pool->max_thread_num)
    {
        return 1;
    }

    return 0;
}

//节点设备的任务类型是否超过该类型任务最大占用线程
static int
tf_thread_pool_node_worker_thread_is_busy(
                                                THREAD_POOL      *p_thread_pool, 
                                                TF_NODE_WORKER  *p_worker)
{
    THREAD_WORKER_TYPE_INFO *p_type_info;

    p_type_info = &p_thread_pool->type_info[p_worker->worker_type];
    
    if(p_type_info->thread_limit && (p_type_info->thread_limit <= p_type_info->thread_cnt))
    {   
        return 1;
    }

    return 0;
}

//每次从编号最低onu开始遍历
static TF_NODE_WORKER*
tf_thread_pool_node_worker_get(
                                    THREAD_POOL   *p_thread_pool, 
                                    uint32_t      *p_key)
{
    int32_t                 port_idx, byte_idx, bit_idx;
    TF_NODE_WORKER         *p_worker;

    for(port_idx = 0; port_idx < MAX_WORKER_GROUP; port_idx++)
    {
        for(byte_idx = 0; byte_idx < MAX_NODE_EACH_PORT/8; byte_idx++)
        {
            if(p_thread_pool->worker_bitmap[port_idx][byte_idx] == 0)
            {
                continue;
            }
                
            for(bit_idx = 7; bit_idx >= 0; bit_idx--)
            {
                if((p_thread_pool->worker_bitmap[port_idx][byte_idx] >> bit_idx) & 0x01U)
                {
                    *p_key = PHY_ID_2_PHY_UINT(port_idx, byte_idx*8+7-bit_idx);
                    
                    //节点的任务是否正在执行
                    if (tf_thread_pool_node_worker_is_busy(p_thread_pool, *p_key))
                    {
                         continue;//遍历下一个节点
                    }
                    
                    p_worker = STAILQ_FIRST(&p_thread_pool->p_node_worker_list[*p_key]);

                    //该类型任务是否超过最大线程数
                    if (tf_thread_pool_node_worker_thread_is_busy(p_thread_pool, p_worker))
                    {
                        continue;//遍历下一个节点
                    }
                    
                    //节点的任务取出
                    STAILQ_REMOVE_HEAD(&p_thread_pool->p_node_worker_list[*p_key], next);
                    if (STAILQ_EMPTY(&p_thread_pool->p_node_worker_list[*p_key]))
                    {
                        //刷新数图
                        tf_net_bitlist_clear(p_thread_pool->worker_bitmap[port_idx], byte_idx*8+7-bit_idx);
                    }
                    
                    return p_worker;
                }
            }
        }
    }

    return NULL;
}

#if 0
//每次遍历onu依赖于前次
static TF_NODE_WORKER*
tf_thread_pool_node_worker_polling_get(
                                    THREAD_POOL  *p_thread_pool, 
                                    uint32_t     *p_key)
{
    static int32_t          port_idx = 0, byte_idx = 0, bit_idx = 7;
    TF_NODE_WORKER         *p_worker;

    for(; port_idx < MAX_WORKER_GROUP; port_idx++)
    {
        for(; byte_idx < MAX_NODE_EACH_PORT/8; byte_idx++)
        {
            if(p_thread_pool->worker_bitmap[port_idx][byte_idx] == 0)
            {
                continue;
            }
                
            for(; bit_idx >= 0; bit_idx--)
            {
                if((p_thread_pool->worker_bitmap[port_idx][byte_idx] >> bit_idx) & 0x01U)
                {
                    *p_key = PHY_ID_2_PHY_UINT(port_idx, byte_idx*8+7-bit_idx);
                    
                    //节点的任务是否正在执行
                    if (tf_thread_pool_node_worker_is_busy(p_thread_pool, *p_key))
                    {
                         continue;//遍历下一个节点
                    }
                    
                    p_worker = STAILQ_FIRST(&p_thread_pool->p_node_worker_list[*p_key]);

                    //该类型任务是否超过最大线程数
                    if (tf_thread_pool_node_worker_thread_is_busy(p_thread_pool, p_worker))
                    {
                        continue;//遍历下一个节点
                    }
                    
                    //节点的任务取出
                    STAILQ_REMOVE_HEAD(&p_thread_pool->p_node_worker_list[*p_key], next);
                    if (STAILQ_EMPTY(&p_thread_pool->p_node_worker_list[*p_key]))
                    {
                        //刷新数图
                        tf_net_bitlist_clear(p_thread_pool->worker_bitmap[port_idx], byte_idx*8+7-bit_idx);
                    }
                    
                    return p_worker;
                }
            }
            bit_idx = 7;
        }
        byte_idx = 0;
    }
    
    port_idx = 0;
    
    return NULL;
}
#endif

static void *
tf_thread_pool_thread_routine (
                void *arg)
{
    THREAD_POOL             *p_thread_pool = (THREAD_POOL *)arg;
    TF_NODE_WORKER         *p_worker;
    const pthread_t         self_thread_id = pthread_self();
    uint16_t                self_idx;
    uint16_t                idx;
    THREAD_WORKER_TYPE_INFO *p_type_info;
    uint32_t                key;
    
    for(idx = 0; idx < p_thread_pool->max_thread_num; idx++)
    {
        if(self_thread_id == p_thread_pool->p_thread_id_queue[idx])
        {
            self_idx = idx;
            break;
        }
    }
    
    if(idx == p_thread_pool->max_thread_num)
        pthread_exit (NULL);

    pthread_mutex_lock(&(p_thread_pool->worker_list_lock));

    while (1)
    {
        if (p_thread_pool->shutdown_flag)
        {
            pthread_mutex_unlock(&(p_thread_pool->worker_list_lock));

            pthread_exit (NULL);
        }
             
        p_worker = tf_thread_pool_node_worker_get(p_thread_pool, &key);
    
        if(!p_worker)
        {
            //printf("%s:%d pid %3d ----------- sleep.\r\n", __FUNCTION__, __LINE__, self_idx);
            pthread_cond_wait(&(p_thread_pool->worker_list_cond), &(p_thread_pool->worker_list_lock));
            //printf("%s:%d pid %3d ----------- wake up.\r\n", __FUNCTION__, __LINE__, self_idx);
            continue;
        }

        p_type_info = &p_thread_pool->type_info[p_worker->worker_type];
        p_thread_pool->p_processing_key_queue[self_idx] = key;
        p_type_info->thread_cnt += 1;
        
        pthread_mutex_unlock(&(p_thread_pool->worker_list_lock));

        if(!p_worker->remove_flag)
        {
            //任务链节点ID还原无效onu物理ID
            KEY2PHYUINT_KEY(key);
            
            printf("%s:%d pid %3d ----------- process key:%d type:%d arg:%d\r\n", __FUNCTION__, __LINE__, self_idx, key, p_worker->worker_type, p_worker->arg1);
            (*(p_worker->handler))(key, p_worker->arg1);
        }

        free (p_worker);

        p_thread_pool->p_processing_key_queue[self_idx] = INVALID_KEY;
        p_type_info->thread_cnt -= 1;
        p_type_info->worker_cnt -= 1;
        if(!p_type_info->worker_cnt && p_type_info->release_cb)
            p_type_info->release_cb();

        pthread_mutex_lock(&(p_thread_pool->worker_list_lock));
    }

    pthread_exit (NULL);
}

static void
tf_thread_pool_node_worker_destory(
                THREAD_POOL *p_thread_pool)
{
    int32_t         port_idx, byte_idx, bit_idx;
    uint32_t        key;
    TF_NODE_WORKER *p_worker, *p_tmp_worker;

    //0~15 16
    for(port_idx = 0; port_idx < MAX_WORKER_GROUP; port_idx++)
    {
        for(byte_idx = 0; byte_idx < MAX_NODE_EACH_PORT/8; byte_idx++)
        {
            if(p_thread_pool->worker_bitmap[port_idx][byte_idx] == 0)
            {
                continue;
            }
                
            for(bit_idx = 7; bit_idx >= 0; bit_idx--)
            {
                if((p_thread_pool->worker_bitmap[port_idx][byte_idx] >> bit_idx) & 0x01U)
                {
                    key = PHY_ID_2_PHY_UINT(port_idx, byte_idx*8+7-bit_idx);
                    STAILQ_FOREACH_SAFE(p_worker, &p_thread_pool->p_node_worker_list[key], next, p_tmp_worker)
                    {
                        free (p_worker); 
                    }
                }
            }
        }
    }
    
    return ;
}

int
tf_thread_pool_destory(THREAD_POOL *p_thread_pool)
{
    uint32_t        idx;

    if (p_thread_pool->shutdown_flag)
        return -1;

    if (p_thread_pool->p_thread_id_queue == NULL)
        return 0;

    p_thread_pool->shutdown_flag = 1;

    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast(&(p_thread_pool->worker_list_cond));

    /*阻塞等待线程退出，否则就成僵尸了*/
    for (idx = 0; idx < p_thread_pool->max_thread_num; idx++)
        pthread_join(p_thread_pool->p_thread_id_queue[idx], NULL);

    free(p_thread_pool->p_thread_id_queue);
    free(p_thread_pool->p_processing_key_queue);

    /*销毁等待队列*/
    tf_thread_pool_node_worker_destory(p_thread_pool);

    free(p_thread_pool->p_node_worker_list);
    
    for (idx = 0; idx < MAX_WORKER_TYPE_NUM; idx++)
        if (p_thread_pool->type_info[idx].release_cb)
            p_thread_pool->type_info[idx].release_cb();

    p_thread_pool->max_thread_num = 0;

    pthread_mutex_destroy(&(p_thread_pool->worker_list_lock));
    pthread_cond_destroy(&(p_thread_pool->worker_list_cond));

    return 0;
}

void
tf_thread_pool_init (
                THREAD_POOL     *p_thread_pool,
                const uint16_t  max_thread_num)
{
    uint32_t        idx;
    pthread_attr_t  attr;

    if (p_thread_pool->max_thread_num)
    {
        printf("Error: init worker pool repeatedly.\r\n");
        return;
    }
    
    //预留无效phy_id 增加一个虚拟tf口
    p_thread_pool->p_node_worker_list = (TF_NODE_WORKER_LIST*)malloc(sizeof(TF_NODE_WORKER_LIST) * MAX_WORKER_NUM);
    if (!p_thread_pool->p_node_worker_list)
    {
        printf("Error: init worker pool fail.\r\n");
        return; 
    }
    
    for (idx = 0; idx < MAX_WORKER_NUM; idx++)
    STAILQ_INIT(&p_thread_pool->p_node_worker_list[idx]);

    pthread_mutex_init(&p_thread_pool->worker_list_lock, NULL);
    pthread_cond_init(&p_thread_pool->worker_list_cond, NULL);

    p_thread_pool->shutdown_flag = 0;

    p_thread_pool->max_thread_num = max_thread_num;
    p_thread_pool->p_thread_id_queue = (pthread_t *)malloc(max_thread_num * sizeof(pthread_t));
    p_thread_pool->p_processing_key_queue = (uint32_t *)malloc(max_thread_num * sizeof(uint32_t));
    memset(p_thread_pool->p_processing_key_queue, 0xFF, max_thread_num * sizeof(uint32_t));
    memset(p_thread_pool->type_info, 0, sizeof(p_thread_pool->type_info));
    memset(p_thread_pool->worker_bitmap, 0 , sizeof(p_thread_pool->worker_bitmap));

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for (idx = 0; idx < max_thread_num; idx++)
        pthread_create(&(p_thread_pool->p_thread_id_queue[idx]), &attr, tf_thread_pool_thread_routine, (void*)p_thread_pool);

    return;
}


#if 000 // test code, do not remove!!!!
void
test_proc_handle(
                uint32_t key,
                uint32_t resv1)
{
    usleep(500000);
}

void
test_release_cb(void)
{
    printf("mission complete\r\n");
}

void
test_print_list(THREAD_POOL *p_thread_pool)
{
    int32_t  port_idx, byte_idx, bit_idx;
    uint32_t key;
    TF_NODE_WORKER *p_worker;
    
    for(port_idx = 0; port_idx < MAX_WORKER_GROUP; port_idx++)
    {
        for(byte_idx = 0; byte_idx < MAX_NODE_EACH_PORT/8; byte_idx++)
        {
            if(p_thread_pool->worker_bitmap[port_idx][byte_idx] == 0)
            {
                continue;
            }
                
            for(bit_idx = 7; bit_idx >= 0; bit_idx--)
            {
                if((p_thread_pool->worker_bitmap[port_idx][byte_idx] >> bit_idx) & 0x01U)
                {
                    key = PHY_ID_2_PHY_UINT(port_idx, byte_idx*8+7-bit_idx);
                    STAILQ_FOREACH(p_worker, &p_thread_pool->p_node_worker_list[key], next)
                    {
                        printf("key: %d work type %d arg %d remove %d \r\n", key, p_worker->worker_type, p_worker->arg1, p_worker->remove_flag);
                    }
                }
            }
        }
    }  
}



int
main(int argc, const char*argv[])
{
    static THREAD_POOL pt_pool;
    char ch;
    uint32_t seq = 0;

    tf_thread_pool_init(&pt_pool, 3);
    tf_thread_pool_worker_type_set_limit(&pt_pool, 0, 2);
    tf_thread_pool_worker_type_set_release_cb(&pt_pool, 0, test_release_cb);

    while(1)
    {
        ch = getchar();
        switch(ch)
        {
            case '1':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 0, 1, seq++);
                break;
            case '2':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 0, 2, seq++);
                break;
            case '3':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 0, 3, seq++);
                break;
            case '4':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 0, 4, seq++);
                break;

            case '5':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 1, 1, seq++);
                break;
            case '6':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 1, 2, seq++);
                break;
            case '7':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 1, 3, seq++);
                break;
            case '8':
                tf_thread_pool_add_worker(&pt_pool, test_proc_handle, 1, 4, seq++);
                break;

            case '9':
                tf_thread_pool_clear_worker(&pt_pool, 1, WORKER_TYPE_COMMON);
                tf_thread_pool_clear_worker(&pt_pool, 1, WORKER_TYPE_UPGRADE);
                break;

            case 'd':
                tf_thread_pool_destory(&pt_pool);
                break;

            case 'i':
                tf_thread_pool_init(&pt_pool, 2);
                break;

            case 'p':
                test_print_list(&pt_pool);
                break;
        }
    }
}
#endif
