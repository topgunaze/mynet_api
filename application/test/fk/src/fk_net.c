/**************************************************************
* 文件名称: 
* 文件描述:
* 版           本: 
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>

**************************************************************/

#include <malloc.h>
#include <pthread.h>
#include "fk_net.h"
#include "ev.h"
//#include "fk_db.h"
#include "tf_types.h"

UINT32  g_fk_cc_pro_idx[FK_NET_MSG_CC_PRO_NUM] = {0,1,2,3,4,5,6,7,8};

/*变量名称g_fk_net_msg_mp涉及宏，慎重修改*/
//业务板板间消息内存池
static TF_NET_BLK_POOL     g_fk_net_msg_mp;
//业务板收发消息队列消息内存池
static TF_NET_BLK_POOL     g_fk_tr_mq_mp;
//业务板libev 监视器内存池
static TF_NET_BLK_POOL     g_fk_watcher_mp;
//业务板 板同步消息连接内存池
static TF_NET_BLK_POOL     g_fk_conn_mp;
//业务板 板同步消息内存池
static TF_NET_BLK_POOL     g_fk_conn_msg_mp;

//conn 公共锁
static TF_NET_MUTUX        g_fk_conn_lock;

//业务板与控制板之间的同步消息连接
static TF_NET_CONN         *g_fk_conn[TF_CTRL_SUPPORT_MAX_SLOT];

//业务板接收同步请求消息队列
static TF_NET_ZC_MQ      g_fk_rx_syn_req_mq[FK_NET_MSG_CC_PRO_NUM];
//业务板接收同步响应消息队列
static TF_NET_ZC_MQ      g_fk_rx_syn_ack_mq;
//业务板接收异步请求消息队列
static TF_NET_ZC_MQ      g_fk_rx_asyn_req_mq;
//业务板接收异步响应消息队列
static TF_NET_ZC_MQ      g_fk_rx_asyn_ack_mq;
//业务板发送消息队列
static TF_NET_ZC_MQ      g_fk_tx_mq;
//业务板运行状态
TF_FK_STATE         g_fk_state;
//业务板CPU运行时间
TF_NET_CPU_OCCUPY        g_fk_cpu_time;

static unsigned int 
fk_net_syn_req_olt_set_process(TF_NET_MSG *p_msg);
static unsigned int 
fk_net_syn_req_olt_get_process(TF_NET_MSG *p_msg);

static unsigned int
fk_net_asyn_ack_work_process(TF_NET_MSG *p_msg);
static unsigned int
fk_net_asyn_ack_hb_process(TF_NET_MSG *p_msg);
static unsigned int
fk_net_asyn_ack_alarm_process(TF_NET_MSG *p_msg);
static unsigned int 
fk_net_asyn_req_work_send(void);
static unsigned int 
fk_net_asyn_req_hb_send(void);

uint8_t g_fk_packet_test;

#if DEFUNC("业务板工作状态接口")

static void
fk_net_state_init(void)
{
    bzero(&g_fk_state, sizeof(TF_FK_STATE));
    tf_net_mutex_create(&g_fk_state.mutex);
    
    return ;
}

void
fk_net_state_set(const int state)
{  
    tf_net_mutex_lock(&g_fk_state.mutex);
    g_fk_state.state |= state;
    tf_net_mutex_unlock(&g_fk_state.mutex);

    return;
}

uint32_t
fk_net_state_get(void)
{   
    return g_fk_state.state;
}

void
fk_net_state_clear(const int state)
{  
    tf_net_mutex_lock(&g_fk_state.mutex);
    g_fk_state.state &= ~state;
    tf_net_mutex_unlock(&g_fk_state.mutex);

    return;
}

uint32_t
fk_net_state_check(const int state)
{   
    return (g_fk_state.state & state) == state ?  1 : 0;
}

uint32_t
fk_net_state_prepare_work(void)
{
    return fk_net_state_check(FK_DEVICE_STATE_STANDBY);
}

uint32_t
fk_net_state_is_work(void)
{
   return fk_net_state_check(FK_NET_STATE_WORKING); 
}

static void
fk_net_state_net_param_set(const TF_NET_PARA  *p_param)
{  
    tf_net_mutex_lock(&g_fk_state.mutex);
    memcpy(&g_fk_state.net_param, p_param, sizeof(TF_NET_PARA));
    tf_net_mutex_unlock(&g_fk_state.mutex);

    return;
}

#endif

#if DEFUNC("业务板网络断链事件处理接口")

static void
fk_net_inactive_disconnect_clear(EV_P_ struct ev_io *watcher)
{
    //1.释放tcp连接资源；
    close(watcher->fd);
    //2.释放连接对应的读事件
    ev_io_stop(loop, watcher); 
    tf_net_safe_free(watcher);
    //3.清空对应slot的网络状态
    fk_net_state_clear(FK_NET_CONNECTION_COMPLETED|FK_NET_WORK_SYNCHRONOUS|FK_NET_STATE_WORK_WAITED);

    return ;
}

#endif

#if DEFUNC("消息队列操作接口")

//接收板间消息异步分发处理
static void 
fk_net_msg_asyn_mq_put(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = p_os_msg->data; 
    
    switch(p_usr_msg ->direction)
    {
        case NET_MSG_DRICTION_REQUEST:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_asyn_req_mq, p_os_msg);
            break;

        case NET_MSG_DRICTION_ACK:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_asyn_ack_mq, p_os_msg);
            break;

        default:  
            tf_net_zc_mq_os_msg_free(p_os_msg);      
            printf( "%s %s %d tf drv not support msg direction!!! \r\n", __FILE__, __FUNCTION__, __LINE__);
            break;
    }
  
    return ;
}

//接收板间同步请求消息负载均衡方式入队列
static void
fk_net_msg_syn_req_mq_put(TF_NET_OS_MSG *p_os_msg)
{
    //无序消息以负载均衡方式处理
    TF_NET_MSG     *p_usr_msg = p_os_msg->data;
    uint32_t        is_order   = p_usr_msg->cmd_info.order;
    static uint8_t  poll_id    = 0;

    switch(is_order)
    {
    
#if 0    
        case NET_MSG_ORDER_IN:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_syn_req_mq[FK_NET_MSG_SER_PRO_ID], p_os_msg);
            break;
#endif    

        case NET_MSG_ORDER_OUT_OF:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_syn_req_mq[poll_id], p_os_msg);
            ++poll_id;
            poll_id &= FK_NET_MSG_CC_PRO_MASK;
            break;

        default:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_syn_req_mq[FK_NET_MSG_SER_PRO_ID], p_os_msg);
            break;
            
#if 0
        default:
            tf_net_zc_mq_os_msg_free(p_os_msg);     
            printf( "%s %s %d tf drv not support msg order!!! \r\n", __FILE__, __FUNCTION__, __LINE__);
            break;
#endif            
    }

    return ;
}

//接收板间同步消息分发处理
static void 
fk_net_msg_syn_mq_put(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = p_os_msg->data;
    
    switch(p_usr_msg ->direction)
    {
        case NET_MSG_DRICTION_REQUEST:
            fk_net_msg_syn_req_mq_put(p_os_msg);
            break;

        case NET_MSG_DRICTION_ACK:
            tf_net_zc_mq_os_msg_put(&g_fk_rx_syn_ack_mq, p_os_msg);
            break;

        default:
            tf_net_zc_mq_os_msg_free(p_os_msg);     
            printf( "%s %s %d tf drv not support msg direction!!! \r\n", __FILE__, __FUNCTION__, __LINE__);
            break;
    }

    return ;
}

//接收板间消息按类型入队列,入参合法性由上级保证
static void 
fk_net_rx_mq_dispatch(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = (TF_NET_MSG*)p_os_msg->data;
    
    if (NET_MSG_ASYN == p_usr_msg->syn_flag)
    {
        fk_net_msg_asyn_mq_put(p_os_msg);
    }
    else
    {
        fk_net_msg_syn_mq_put(p_os_msg);
    }

    return ;
}

//接收板间消息入队列,入参合法性由上级保证
static void 
fk_net_rx_mq_put(TF_NET_MSG *data)
{
    TF_NET_OS_MSG   *p_msg;
    
    //该申请bcmos msg由队列接收端释放
    p_msg = (TF_NET_OS_MSG*)tf_net_malloc(&g_fk_tr_mq_mp, sizeof(TF_NET_OS_MSG));
    if (!p_msg)
    {
        tf_net_safe_free(data);
        printf( "%s %s %d malloc fail\r\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    p_msg->data    = data;
    p_msg->size    = data->len;//队列中的消息体长度为板间消息有效长度

    //消息分发队列，整个过程由os接口托管
    fk_net_rx_mq_dispatch(p_msg);

    return;
}

//发送板间消息入队列,入参合法性由上级保证
static unsigned int 
fk_net_tx_mq_put(TF_NET_MSG *data)
{
    TF_NET_OS_MSG  *p_msg;
    //申请msg内存由队列接收端释放
    p_msg = (TF_NET_OS_MSG*)tf_net_malloc(&g_fk_tr_mq_mp, sizeof(TF_NET_OS_MSG));
    if (!p_msg)
    {
        tf_net_safe_free(data);
        printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return FK_ZK_RC_MEM_ALLOCATION;
    }

    //data 字段存储板间消息体
    p_msg->data = data;
    p_msg->size = data->len;
    
    tf_net_zc_mq_os_msg_put(&g_fk_tx_mq, p_msg);

    return 0;
}

#endif

#if DEFUNC("业务板同步消息处理接口")

//接收同步请求消息处理函数组
static net_msg_handler fk_net_syn_req_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            NULL, NULL,
            fk_net_syn_req_olt_set_process,
            fk_net_syn_req_olt_get_process,
            NULL, NULL, NULL}; 
            
//处理控制板同步请求消息
static unsigned int 
fk_net_msg_syn_req_process(TF_NET_MSG *p_msg)
{
    unsigned int rc = 0;

    if (fk_net_syn_req_handler[p_msg->msg_type])
    {
        if ((rc = fk_net_syn_req_handler[p_msg->msg_type](p_msg)))
        {
            printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }
    }
    else
    {
        printf( "%s %s %d syn req tf drv not support msg type %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, p_msg->msg_type);
    }
    
    return rc;
}

//同步消息fifo 申请
static uint32_t 
fk_net_conn_msg_list_alloc(TF_NET_CONN *conn)
{
    int i;
    TF_NET_CONN_MSG *ptmsg;
    
    conn->msg_array = tf_net_calloc(&g_fk_conn_msg_mp, conn->msg_num, 
                        sizeof(TF_NET_CONN_MSG));
   
    if (!conn->msg_array)
        return FK_ZK_RC_MEM_ALLOCATION;

    ptmsg = conn->msg_array;
    for (i = 0; i < conn->msg_num; i++, ptmsg++)
    {
        uint32_t rc;

        TAILQ_INSERT_TAIL(&conn->free_req_list, ptmsg, l);
        
        rc = tf_net_sem_create(&ptmsg->sem, 0);
        if (rc)
            return rc;
    }

    return 0;
}

static uint32_t 
fk_net_conn_init(uint32_t slot_id, TF_NET_CONN **pconn)
{
    TF_NET_CONN *conn;

    tf_net_mutex_lock(&g_fk_conn_lock);
    conn = tf_net_calloc(&g_fk_conn_mp, 1, sizeof(*conn));
    
    if (!conn)
    {
        tf_net_mutex_unlock(&g_fk_conn_lock);
        return FK_ZK_RC_MEM_ALLOCATION;
    }

    snprintf(conn->name, sizeof(conn->name), "2tfctrl_slot_%u", slot_id);
    TAILQ_INIT(&conn->free_req_list);
    TAILQ_INIT(&conn->msg_list);
    tf_net_mutex_create(&conn->mutex);
    conn->msg_num = FK_NET_CONN_MSG_NUM;

    fk_net_conn_msg_list_alloc(conn);

    conn->connected = 1;
    conn->slot_id = slot_id;
    *pconn = conn;
    
    tf_net_mutex_unlock(&g_fk_conn_lock);

    return 0;
}

static uint32_t 
fk_net_conn_get_any(uint32_t slot_id, TF_NET_CONN **pconn)
{
    uint32_t err;

    if (!pconn)
    {
        return FK_ZK_RC_PARAM_NULL;
    }
    
    if (slot_id >= TF_CTRL_SUPPORT_MAX_SLOT)
    {
        return FK_ZK_RC_PARAM_OUT_OF_RANGE;
    }
    
    *pconn = g_fk_conn[slot_id];
    
    if (*pconn)
    {
        return 0;
    }
    
    err = fk_net_conn_init(slot_id, &g_fk_conn[slot_id]);
    *pconn = g_fk_conn[slot_id];
    
    return err;
}

static uint32_t 
fk_net_conn_get(uint32_t slot_id, TF_NET_CONN **pconn)
{
    return fk_net_conn_get_any(slot_id, pconn);
}

//同步消息发送
static uint32_t 
fk_net_msg_syn_send(TF_NET_MSG *pmsg)
{
    if (!pmsg)
    {
        return FK_ZK_RC_PARAM_NULL;
    }
    
    TF_NET_CONN_MSG *pcmsg;
    TF_NET_CONN     *pconn;
    unsigned int     err;

    //根据slot获取conn, 业务面默认使用0
    err = fk_net_conn_get(0, &pconn);
    if (err)
    {
        printf( "fk_net_conn_get error!!!\r\n");
        return err;
    }

    /* request under connection lock */
    tf_net_mutex_lock(&pconn->mutex);

    pcmsg = tf_net_conn_msg_get_free(&pconn->free_req_list); 
    if (!pcmsg)
    {
        tf_net_mutex_unlock(&pconn->mutex);
        printf( "fk_net_conn_msg_get_free error!!!\r\n");
        return FK_ZK_RC_PARAM_GET;
    }

    //时间戳作为接收校验魔术字
    pmsg->corr_tag = tf_net_timestamp();
    
    //拷贝用于发送消息，本体用于同步消息接收
    TF_NET_MSG *p_autofree_msg = tf_net_malloc(&g_fk_net_msg_mp, pmsg->len);
    if (!p_autofree_msg)
    {
        printf( "%s %s %d tf_net_malloc error!!!\r\n", __FILE__, __FUNCTION__, __LINE__);
        tf_net_conn_msg_free(pcmsg, &pconn->free_req_list);
        return FK_ZK_RC_MEM_ALLOCATION;
    }
    
    memcpy(p_autofree_msg, pmsg, pmsg->len);
    
    //pmsg挂conn 消息用于接收
    pcmsg->data = pmsg;

    //发送任务,bcmos 队列在发送完后会自动释放bcmos msg
    if ((err = fk_net_tx_mq_put(p_autofree_msg)))
    {
        printf( "%s %s %d error!!!\r\n", __FILE__, __FUNCTION__, __LINE__);
        //pmsg 由外部释放
        //pcmsg 释放到空闲队列
        tf_net_conn_msg_free(pcmsg, &pconn->free_req_list);
        return err;
    }

    //进入等待消息队列
    TAILQ_INSERT_TAIL(&pconn->msg_list, pcmsg, l);
    
    tf_net_mutex_unlock(&pconn->mutex);

    /* Wait for restfse or timeout*/
    if (tf_net_sem_wait(&pcmsg->sem, pmsg->cmd_info.timeout))
    {
        //pmsg 由外部释放
        //pcmsg 不再关注需要释放到空闲队列
        tf_net_mutex_lock(&pconn->mutex);
        
        TAILQ_REMOVE(&pconn->msg_list, pcmsg, l);
        tf_net_conn_msg_free(pcmsg, &pconn->free_req_list);
        
        tf_net_mutex_unlock(&pconn->mutex);
        
        return FK_ZK_RC_MSG_SYN_WAITE;
    }

    //正常得到回应入空闲队列
    tf_net_mutex_lock(&pconn->mutex);
    tf_net_conn_msg_free(pcmsg, &pconn->free_req_list);
    tf_net_mutex_unlock(&pconn->mutex);

    return 0;
}

/*
接口包含三种返回值：
1. 业务板接口  返回  state
2. 接口                   返回  return
3. 出参                   返回  param
*/

uint32_t 
fk_net_syn_operation( 
                                IN const TF_NET_MSG_TYPE  msg_type,
                                IN const  uint64_t         vif_info, 
                                IN const  TF_NET_MSG_CMD  cmd_info, 
                                IN const  void             *pvar, 
                                IN const  uint32_t         pvar_len,
                                OUT void                   *param,
                                IN  const uint32_t         param_len,
                                OUT uint32_t               *state)
{
    uint32_t        syn_ack_len;
    TF_NET_MSG     *p_msg;
    uint32_t        rc;
    tf_rc_info     rc_info = {.obj_id = OBJ_DRV_FK, .sub_obj_id.olt_id = (vif_info & 0xff)};

    //判断与控制面的通信是否已经建立
    if (!fk_net_state_is_work())
    {
        return TF_OBJ_DRV_TF_OLT_RC_OFFSET + FK_ZK_RC_NET_NOT_WORK;
    }
  
    if (!state)
    {
        TF_TF_ERRNO_INFO2RC(rc_info, rc, FK_ZK_RC_PARAM_NULL);
        return rc;
    }
    
    if ((param && 0 == param_len) || (NULL == param && param_len))
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        TF_TF_ERRNO_INFO2RC(rc_info, rc, FK_ZK_RC_PARAM_OUT_OF_RANGE);
        return rc;
    }
    
    if ((NULL == param && 0 == param_len) || (param_len < pvar_len))
    {
        //需要传参但不需要返回参数|| 传参结构大于返回参数
        p_msg = tf_net_msg_pack(&g_fk_net_msg_mp, vif_info, cmd_info, pvar, pvar_len, pvar_len);
    }
    else
    {
        //传参结构小于返回参数
        p_msg = tf_net_msg_pack(&g_fk_net_msg_mp, vif_info, cmd_info, pvar, pvar_len, param_len);
    }
   
    if (!p_msg)
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        TF_TF_ERRNO_INFO2RC(rc_info, rc, FK_ZK_RC_MEM_ALLOCATION);
        return rc;
    }
    
    p_msg->msg_type  = msg_type;
    p_msg->direction = NET_MSG_DRICTION_REQUEST;
    p_msg->syn_flag  = NET_MSG_SYN;

    //msg 由同步接口释放
    if ((rc = fk_net_msg_syn_send(p_msg)))
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        tf_net_safe_free(p_msg);
        TF_TF_ERRNO_INFO2RC(rc_info, rc, rc);
        return rc;
    }
    
    *state = p_msg->state;

    if (!(*state))
    {
        if(param && param_len)
        {
            syn_ack_len = p_msg->len - NET_MSG_HEAD_LEN;
            
            if (param_len < syn_ack_len)
                printf( "tf drv struct msg, body size too small!!!\r\n");

            memcpy(param, p_msg->body, (syn_ack_len > param_len ? param_len : syn_ack_len));
        }
    }
        
    tf_net_safe_free(p_msg);

    return 0;  
}

#endif

#if DEFUNC("业务板异步消息处理接口")

//业务板接收异步请求消息处理函数组
static net_msg_handler fk_net_asyn_req_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            NULL, 
            NULL,
            NULL, NULL, NULL, NULL, NULL, NULL,
            NULL,
            NULL,NULL}; 

//业务板接收异步响应消息处理函数组
static net_msg_handler fk_net_asyn_ack_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            fk_net_asyn_ack_work_process, 
            fk_net_asyn_ack_hb_process,
            NULL, NULL, NULL, NULL, NULL, NULL,
            fk_net_asyn_ack_alarm_process,
            NULL}; 

//处理控制板响应
static unsigned int 
fk_net_msg_asyn_ack_process(TF_NET_MSG *p_msg)
{
    unsigned int rc = 0;

    if (fk_net_asyn_ack_handler[p_msg->msg_type])
    {
        if ((rc = fk_net_asyn_ack_handler[p_msg->msg_type](p_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }
    }
    else
    {
        printf( "%s %s %d asyn ack tf drv not support msg type %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, p_msg->msg_type);
    }

    return rc;
}

//处理控制板响应
static unsigned int 
fk_net_msg_asyn_req_process(TF_NET_MSG *p_msg)
{
    unsigned int rc = 0;

    if (fk_net_asyn_req_handler[p_msg->msg_type])
    {
        if ((rc = fk_net_asyn_req_handler[p_msg->msg_type](p_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }
    }
    else
    {
        printf( "%s %s %d req tf drv not support %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, p_msg->msg_type);  
    }

    return rc;
}

#endif

#if DEFUNC("网络任务基本接口")

//消息发送接口
static unsigned int 
fk_net_msg_send( 
                            TF_NET_MSG         *pmsg,
                            const void          *pvar, 
                            const unsigned int  len,
                            const int           state,
                            const int           direction)
{
    TF_NET_MSG      *ptmsg;
    unsigned int     rc;

    if (len)
    {
        ptmsg = tf_net_msg_pack(&g_fk_net_msg_mp, pmsg->vif_info, pmsg->cmd_info, pvar, len, len);
    }
    else
    {
        ptmsg = tf_net_msg_pack(&g_fk_net_msg_mp, pmsg->vif_info, pmsg->cmd_info, NULL, 0, 0);  
    }

    /*ptmsg 内存由发送端或者发送队列接口释放需要保证body_size 与 len一致*/
    if (!ptmsg)
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return FK_ZK_RC_MSG_PACK;
    }
    
     ptmsg->msg_type  = pmsg->msg_type;
     ptmsg->direction = direction;
     ptmsg->syn_flag  = pmsg->syn_flag;
     ptmsg->corr_tag  = pmsg->corr_tag;
     ptmsg->state     = state;
    
     if ((rc = fk_net_tx_mq_put(ptmsg)))
     {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;  
     }
    
     return 0;
}

//业务板网络接口初始化
static unsigned int 
fk_net_init(int *p_fd, TF_NET_PARA *p_net_param)
{
    struct sockaddr_in sin;
    int                addrlen = sizeof(struct sockaddr);
    int                fd;
    unsigned int       rc;
    
    if (!p_fd || !p_net_param)
    {
        return FK_ZK_RC_PARAM_NULL;
    }
    
    if ((rc = tf_net_init(&sin, &fd)))
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    }

    //本板
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(CLIENT_PORT + g_fk_state.slot_id);

    if (bind(fd, (struct sockaddr *)&sin, addrlen) < 0)
    {
        close(fd);
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return FK_ZK_RC_NET_BIND;
    }

    *p_fd = fd;

    //配置文件获取对端信息,暂时用本板
    p_net_param->remote_addr.sin_port        = htons(SERVER_PORT);
    p_net_param->remote_addr.sin_addr.s_addr = sin.sin_addr.s_addr;
    p_net_param->remote_addr.sin_family      = AF_INET;
 
    return 0;
}

//read 数据事件回调
static void 
fk_net_read_cb(EV_P_ struct ev_io *watcher, int revents)
{  
    TF_NET_MSG  msg_header;
    TF_NET_MSG  *p_msg = NULL;
    unsigned int rc;

    if(EV_ERROR & revents)  
    {  
      printf( "tf drv read event error revents %d\r\n", revents);
      return;
    }

    //1. 预读取头
    if ((rc = tf_net_rec_packet_fix_peek(watcher->fd, &msg_header, NET_MSG_HEAD_LEN)))
    {
        goto fail;
    }

#if 0
    printf("drv rx cb msg info peek:\n"
    "msg_type: %d\n"
    "cmd_id: %d\ntimeout: %d\norder: %d\n" 
    "vif_info: %lld\n"
    "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
    "state: %d\n"
    "len: %d\n"
    "compress: %d\n"
    "body_size: %d\n",
    msg_header.msg_type,
    msg_header.cmd_info.cmd_id,
    msg_header.cmd_info.timeout,
    msg_header.cmd_info.order,
    msg_header.vif_info,
    msg_header.syn_flag,
    msg_header.corr_tag,
    msg_header.direction, 
    msg_header.state, 
    msg_header.len,
    msg_header.compress,
    msg_header.body_size);
#endif

    //2. 头部大小端转换
    
    /*
    构造消息体 ,需要接收的消息长度为头部记录中的len长度，
    内存池中申请消息结构空间固定，
    申请的此段内存由rx队列接收端随bcmos msg的释放而释放
    */
    p_msg = (TF_NET_MSG*)tf_net_malloc(&g_fk_net_msg_mp, msg_header.len);
    if (!p_msg)
    {
        printf( "%s %s %d errorno %d\r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        goto fail;
    }
    
    //3. 预读取消息体，消息体的长度为msg的有效长度与头长之差
    if ((rc = tf_net_rec_packet_fix_peek(watcher->fd, p_msg->body, (msg_header.len - NET_MSG_HEAD_LEN))))
    {
        goto fail;
    }

    //4. 从缓存中取消息
    if ((rc = tf_net_rec_packet_fix(watcher->fd, p_msg, msg_header.len)))
    {
        goto fail;
    }

    //5. 头部大小端转换

    //6. 消息头部校验
    if (tf_net_msg_verify(p_msg))
    {
        //丢弃整个消息    
        printf( 
                "tf drv read cb msg verify error!!!:\n"
                        "msg_type: %d\n"
                        "cmd_id: %d\ntimeout: %d\norder: %d\n" 
                        "vif_info: %lld\n"
                        "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
                        "state: %d\n"
                        "len: %d\n"
                        "compress: %d\n"
                        "body_size: %d\r\n",
                        p_msg->msg_type,
                        p_msg->cmd_info.cmd_id,
                        p_msg->cmd_info.timeout,
                        p_msg->cmd_info.order,
                        p_msg->vif_info,
                        p_msg->syn_flag,
                        p_msg->corr_tag,
                        p_msg->direction, 
                        p_msg->state, 
                        p_msg->len,
                        p_msg->compress,
                        p_msg->body_size);
 
        goto fail;
    }
    
#if 0
    printf("drv rx cb msg info no peek:\n"
    "msg_type: %d\n"
    "cmd_id: %d\ntimeout: %d\n" 
    "vif_info: %lld\n"
    "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
    "state: %d\n"
    "len: %d\n"
    "compress: %d\n"
    "body_size: %d\n",
    msg_header.msg_type,
    msg_header.cmd_info.cmd_id,
    msg_header.cmd_info.timeout,
    msg_header.vif_info,
    msg_header.syn_flag,
    msg_header.corr_tag,
    msg_header.direction, 
    msg_header.state, 
    msg_header.len,
    msg_header.compress,
    msg_header.body_size);
#endif

    if (tf_net_msg_uncompress_process(&g_fk_net_msg_mp, &p_msg))
    {
        printf("tf drv rx cb uncompress fail\r\n");
        goto fail;
    }

    //入接收消息队列,msg内部托管
    fk_net_rx_mq_put(p_msg);

    return;
    
fail:

    tf_net_safe_free(p_msg);

    //断开链接的处理,停掉evnet,同时释放记录客户端w结构体
    if(rc == TF_MDW_NET_RC_REMOTE_DISCONNECT)  
    {  
        fk_net_inactive_disconnect_clear(loop, watcher);
    }
    else
    if (rc)
    {
        printf( "tf drv read error!!!\r\n");
    }

    return;
}

//与控制板建立连接
static unsigned int
fk_net_connect(EV_P)
{
    int             fd;
    TF_NET_PARA     net_param;
    int             addrlen = sizeof(struct sockaddr);
    ev_io           *client_read;
    unsigned int    rc;
    
    if ((rc = fk_net_init(&fd, &net_param)))
    {
        printf( "%s %s %d errorno %d\r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return rc;
    }
    
    if (connect(fd, (struct sockaddr *)&net_param.remote_addr, addrlen))
    {
        printf( "%s %s %d errorno %d\r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        close(fd);
        return FK_ZK_RC_NET_CONNCET;
    }
    
    client_read = (struct ev_io*)tf_net_malloc(&g_fk_watcher_mp, sizeof(struct ev_io));

    if (!client_read)
    {
        close(fd);
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return FK_ZK_RC_MEM_ALLOCATION;
    }

    fk_net_state_set(FK_NET_CONNECTION_COMPLETED);
    printf("tf drv connetion complete\r\n");

    //read事件
    ev_io_init(client_read, fk_net_read_cb, fd, EV_READ);  
    ev_io_start(EV_A, client_read);

    net_param.loop    = EV_A;
    net_param.watcher = client_read;
    fk_net_state_net_param_set(&net_param);

    return 0;
}

//周期性开工消息、心跳、断链重连
static void
fk_net_state_machine_cb (EV_P_ ev_timer *w, int revents)
{  
    //printf("fk_net_state_machine_cb %d\r\n", fk_net_state_get());
    
    switch(fk_net_state_get())
    {
        case FK_DEVICE_STATE_INIT:
        case FK_DEVICE_STATE_READY:
            //建立板间连接
            if(fk_net_connect(EV_A))
            {
                printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
            }
            break;

        case FK_NET_CONNECTION_COMPLETED:
            {
               //板间连接已经建立等待设备ready
            }
            break;
            
        case (FK_DEVICE_STATE_STANDBY):
            //开工通知
            if (fk_net_asyn_req_work_send())
            {
                printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
            }
            fk_net_state_set(FK_NET_STATE_WORK_WAITED);
            break;

        case (FK_NET_STATE_WORKING):
            //定时心跳
            if (fk_net_asyn_req_hb_send())
            {
                printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
            }
            break;

        default:
            printf( "%s %s %d tf drv state: %d not support!!!\r\n", __FILE__, __FUNCTION__, __LINE__, fk_net_state_get());
            break;
    }

    return;
}

static void 
fk_net_work_main(void)
{
    struct ev_loop  *loop = ev_default_loop(0);  
    ev_timer        period_watcher;

    //与控制板建立连接，失败后在状态机中重连
    if (fk_net_connect(EV_A))
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
    }

    //定时事件
    ev_timer_init (&period_watcher, fk_net_state_machine_cb, 2, 2);
    ev_timer_start (EV_A, &period_watcher);
  
    ev_loop(EV_A, 0); 
}

//网络通信使用单独线程
static unsigned int 
fk_net_work_init(void)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)fk_net_work_main, NULL) < 0)
    {
        printf( "%s %s %d param error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return FK_ZK_RC_TASK_CREATE;
    }

    return 0;
}

//获取单板网络相关配置，预留接口
static unsigned int 
fk_net_cfg_init(void)
{
    char            value[128] = {0};
    unsigned int    rc;
    
    //1.读配置文件
    if (!(rc = tf_net_config_file_paser("tfdrv.ini", "slot", value, sizeof(value))))
    {
        //2. 初始化全局参数
        g_fk_state.slot_id = atoi(value);
        printf("get ini the slot %d \r\n", g_fk_state.slot_id);
    }

    return rc;
}

//业务板内存池初始化
unsigned int 
fk_net_mempool_init(void)
{
    //接收发送线程消息队列消息内存池
    TF_NET_BLK_POOL_PARM net_mq_parm = { "fk_tr_mq_mp",
                                            sizeof(TF_NET_OS_MSG),
                                            NET_MSG_MAX_COUNT};
                                   
    TF_NET_BLK_POOL_PARM net_msg_parm = {  "fk_net_msg_mp",
                                            NET_BUFFER_LEN,//(sizeof(TF_NET_MSG) + 2048),
                                            NET_MSG_MAX_COUNT};

    TF_NET_BLK_POOL_PARM net_watcher_parm = {  "fk_net_watcher_mp",
                                                sizeof(union ev_any_watcher),
                                                NET_WATCHER_COUNT};
                                                
    TF_NET_BLK_POOL_PARM net_conn_parm    = {  "fk_net_conn_mp",
                                                sizeof(TF_NET_CONN),
                                                TF_CTRL_SUPPORT_MAX_SLOT};

    TF_NET_BLK_POOL_PARM net_conn_msg_parm = {  "fk_net_conn_msg_mp",
                                                 (sizeof(TF_NET_CONN_MSG)*FK_NET_CONN_MSG_NUM),
                                                 TF_CTRL_SUPPORT_MAX_SLOT};
                                                
    unsigned int rc = 0;
    
    rc |= tf_net_blk_pool_create(&g_fk_tr_mq_mp, &net_mq_parm);
    rc |= tf_net_blk_pool_create(&g_fk_net_msg_mp, &net_msg_parm);
    rc |= tf_net_blk_pool_create(&g_fk_watcher_mp, &net_watcher_parm);
    rc |= tf_net_blk_pool_create(&g_fk_conn_mp,     &net_conn_parm);
    rc |= tf_net_blk_pool_create(&g_fk_conn_msg_mp, &net_conn_msg_parm);

    return rc;
}

unsigned int 
fk_net_ev_init(void)
{
    unsigned int rc;
    //网络接口初始化
    
#ifdef TF_NET_USE_MP
    //网络内存池初始化
    if ((rc = fk_net_mempool_init()))
    {     
        printf( "fk_net_mempool_init fail!!!\r\n");
        return rc;
    }
#endif

    //初始系统CPU时间
    tf_net_cpu_time_get(&g_fk_cpu_time);

    fk_net_state_init();

    //初始化配置
    fk_net_cfg_init();
    
    //开辟线程启动异步事件驱动器进行网络监听
    if ((rc = fk_net_work_init()))
    {
        return rc;
    }
    
    return 0;
}

#endif

#if DEFUNC("任务")

//接收同步请求消息处理任务
static void 
fk_net_rx_syn_req_task(void    *p_arg)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    uint32_t        rc;
    uint32_t        idx = *(uint32_t*)p_arg;
    TF_NET_ZC_MQ   *p_mq = &g_fk_rx_syn_req_mq[idx]; 

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(p_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf( "%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;
        
        //处理消息
        if ((rc = fk_net_msg_syn_req_process(p_usr_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//接收同步响应消息处理任务
static void 
fk_net_rx_syn_ack_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    uint32_t        rc;

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_fk_rx_syn_ack_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf( "%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息,业务板默认使用同步连接0
        if ((rc = tf_net_conn_msg_syn_ack_process(g_fk_conn[0], p_usr_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//接收异步请求消息处理任务
static void 
fk_net_rx_asyn_req_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    uint32_t       rc;

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_fk_rx_asyn_req_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf( "%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息
        if ((rc = fk_net_msg_asyn_req_process(p_usr_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//接收同步响应消息处理任务
static void 
fk_net_rx_asyn_ack_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    uint32_t       rc;

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_fk_rx_asyn_ack_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf( "%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息
        if ((rc = fk_net_msg_asyn_ack_process(p_usr_msg)))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//接收同步请求消息队列和任务初始化
static unsigned int 
fk_net_rx_syn_req_task_init(void)
{
    uint32_t            rc;
    pthread_t           tid;
    pthread_attr_t      attr;
    uint32_t            idx;
    char                name[TF_TF_NET_NAME_LEN];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    for (idx = 0; idx < FK_NET_MSG_CC_PRO_NUM; idx++)
    {
        snprintf(name, sizeof(name), "client_rx_syn_req_mq_%d", idx);
        
        rc = tf_net_zc_mq_create(&g_fk_rx_syn_req_mq[idx], name);
        if (rc)
        {
            printf( "tf drv create rx syn req mq error %d idx %d\r\n", rc, idx);
            return FK_ZK_RC_MSG_QUEUE_CREATE;
        }

        if (pthread_create(&tid, &attr, (void *)fk_net_rx_syn_req_task, (void*)&g_fk_cc_pro_idx[idx]) < 0)
        {
            printf( "tf drv create rx syn req task error %d idx %d!!! \r\n", errno, idx);
            return FK_ZK_RC_TASK_CREATE;
        }
    }
    
    return 0;
}

//接收同步响应消息队列和任务初始化
static unsigned int 
fk_net_rx_syn_ack_task_init(void)
{
    uint32_t        rc;
    pthread_t       tid;
    pthread_attr_t  attr;
    
    rc = tf_net_zc_mq_create(&g_fk_rx_syn_ack_mq, "client_rx_syn_ack_mq");
    if (rc)
    {
        printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return FK_ZK_RC_MSG_QUEUE_CREATE;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)fk_net_rx_syn_ack_task, NULL) < 0)
    {
        printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return FK_ZK_RC_TASK_CREATE;
    }

    return 0;
}

//接收同步响应消息队列和任务初始化
static unsigned int 
fk_net_rx_asyn_req_task_init(void)
{
    uint32_t       rc;
    pthread_t      tid;
    pthread_attr_t attr;
    
    rc = tf_net_zc_mq_create(&g_fk_rx_asyn_req_mq, "client_rx_asyn_req_mq");
    if (rc)
    {
        printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return FK_ZK_RC_MSG_QUEUE_CREATE;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)fk_net_rx_asyn_req_task, NULL) < 0)
    {
        printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return FK_ZK_RC_TASK_CREATE;
    }

    return 0;
}

//接收异步响应消息队列和任务初始化
static unsigned int 
fk_net_rx_asyn_ack_task_init(void)
{
    uint32_t       rc;
    pthread_t      tid;
    pthread_attr_t attr;
    
    rc = tf_net_zc_mq_create(&g_fk_rx_asyn_ack_mq, "client_rx_asyn_ack_mq");
    if (rc)
    {
        printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return FK_ZK_RC_MSG_QUEUE_CREATE;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)fk_net_rx_asyn_ack_task, NULL) < 0)
    {
        printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return FK_ZK_RC_TASK_CREATE;
    }

    return 0;
}

//消息发送任务
static void 
fk_net_tx_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    uint32_t       rc;
    TF_NET_MSG     *p_msg;
    
    while (1)
    {
        //m 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_fk_tx_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }
        
        p_msg = (TF_NET_MSG*)p_os_msg->data;

        //发送板间消息到控制板
        if (tf_net_send_alone_packet(g_fk_state.net_param.watcher->fd, p_msg))
        {
            printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, errno); 
        }
        
        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);
    }
    
    return;
}

//消息发送队列与任务初始化
static unsigned int 
fk_net_tx_task_init(void)
{
    uint32_t        rc;
    pthread_t       tid;
    pthread_attr_t  attr;
    
    rc = tf_net_zc_mq_create(&g_fk_tx_mq, "client_tx_queue");
    if (rc)
    {
        printf( "%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return FK_ZK_RC_MSG_QUEUE_CREATE;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)fk_net_tx_task, NULL) < 0)
    {
        printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return FK_ZK_RC_TASK_CREATE;
    }

    return 0;  
}

//网络任务初始化
unsigned int 
fk_net_task_init(void)
{
    unsigned int rc;
    
    if ((rc = fk_net_rx_syn_req_task_init()))
    {
        return rc;
    }

    if ((rc = fk_net_rx_syn_ack_task_init()))
    {
        return rc;
    }

    if ((rc = fk_net_rx_asyn_req_task_init()))
    {
        return rc;
    }

    if ((rc = fk_net_rx_asyn_ack_task_init()))
    {
        return rc;
    }

    if ((rc = fk_net_tx_task_init()))
    {
        return rc;
    }

    return 0;
}

#endif

#if DEFUNC("业务板发送异步请求消息接口")

static unsigned int 
fk_net_asyn_req_hb_send(void)
{
    TF_NET_MSG msg = { NET_MSG_TYPE_HB,
                       {NET_MSG_CMD_HB, },
                       0,
                       NET_MSG_ASYN,
                       0,
                       NET_MSG_DRICTION_REQUEST,
                       0,
                       NET_MSG_HEAD_LEN};
                
    unsigned int      rc;
    TF_NET_CPU_OCCUPY cpu_current_time;
    NIF_LOAD_INFO     load_info;

    tf_net_tf_board2vif_ullong(g_fk_state.slot_id, &msg.vif_info);
    
    if (tf_net_mem_load_get(&load_info.mem_load))
    {
        load_info.mem_load = NET_COM_PARAM_INVALID; 
    }
    
    if (tf_net_cpu_time_get(&cpu_current_time))
    {
        load_info.cpu_load = NET_COM_PARAM_INVALID;
    }
    else
    {
        if (tf_net_cpu_cal_occupy(&g_fk_cpu_time, &cpu_current_time, &load_info.cpu_load))
        {
            load_info.cpu_load = NET_COM_PARAM_INVALID;
        }
        
        memcpy(&g_fk_cpu_time, &cpu_current_time, sizeof(TF_NET_CPU_OCCUPY));
    }

    if ((rc = fk_net_msg_send(&msg, &load_info, sizeof(NIF_LOAD_INFO), 0, msg.direction)))
    {
        printf( "%s %s %d error !!!\r\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return rc;
}

//for test
#if 0
unsigned int 
fk_net_req_asyn_hb_load(void)
{
    //消息体为0
    TF_NET_MSG msg = { NET_MSG_TYPE_HB,
                        {NET_MSG_CMD_HB_LOAD, },
                        0,
                        NET_MSG_ASYN,
                        0,
                        NET_MSG_DRICTION_REQUEST,
                        0,
                        NET_MSG_HEAD_LEN + MSG_BODY_LEN_TEST_PC,
                        MSG_BODY_LEN_TEST_PC};
    unsigned int rc;

    tf_net_tf_board2vif_ullong(0, &msg.vif_info);

    //1. 负荷信息预留
    void *body = (void*)tf_net_malloc(&g_fk_net_msg_mp, MSG_BODY_LEN_TEST_PC);
     
    if (!body)
    {
         printf( "%s %s %d error !!!\r\n", __FILE__, __FUNCTION__, __LINE__);
         return FK_ZK_RC_MEM_ALLOCATION;
    }
    
    memset(body, 0xff, MSG_BODY_LEN_TEST_PC);
    
    if ((rc = fk_net_msg_send(&msg, body, MSG_BODY_LEN_TEST_PC, 0, msg.direction)))
    {
        printf( "%s %s %d error !!!\r\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;  
    }
    
    tf_net_safe_free(body);
    
    return 0;
}
#endif

//开工消息
static unsigned int 
fk_net_asyn_req_work_send(void)
{
    TF_NET_MSG msg = { NET_MSG_TYPE_WORK,
                        {NET_MSG_CMD_WORK, },
                        0,
                        NET_MSG_ASYN,
                        0,
                        NET_MSG_DRICTION_REQUEST,
                        0,
                        NET_MSG_HEAD_LEN,
                        0};
    unsigned int rc;                    
                        
    tf_net_tf_board2vif_ullong(g_fk_state.slot_id, &msg.vif_info);
    
    if ((rc = fk_net_msg_send(&msg, NULL, 0, 0, msg.direction)))
    {
      printf( "%s %s %d error !!!\r\n", __FILE__, __FUNCTION__, __LINE__);
      return rc;  
    }

    return 0;
}

#endif

#if DEFUNC("业务板发送同步请求消息接口")

uint32_t
fk_net_syn_req_profile_get(
                                    const uint8_t   slot_id,
                                    MSG_PC_TEST     *p_in,
                                    MSG_PC_TEST     *p_out)
{
    TF_NET_MSG_CMD     cmd_info;
    uint32_t            state;
    uint32_t            rc;
    TF_NET_PHY_INFO    obj_info = {TF_BOARD, {slot_id}};
    uint64_t            obj_id;

    if((rc = tf_net_phy2vif(&obj_info, &obj_id)))
    {
       printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
       return rc;
    }

    cmd_info.cmd_id     = NET_MSG_CMD_PROFILE_GET;
    cmd_info.timeout    = TF_NET_WAIT_FOREVER;
    cmd_info.order      = NET_MSG_ORDER_IN;

    rc = fk_net_syn_operation(NET_MSG_TYPE_DRV_PULL, obj_id, cmd_info,
            p_in, sizeof(MSG_PC_TEST), p_out, sizeof(MSG_PC_TEST), &state);
 
    return rc ? rc : state;
}

#endif

#if DEFUNC("业务板接收同步请求消息处理接口")

/*
返回码涉及
1. net -> vif phy info error
2. olt -> olt operation error
3. onu -> table info
4. sql -> operation
*/
static unsigned int 
fk_net_syn_req_olt_set_process(TF_NET_MSG *p_msg)
{
    /*参数名称和标志p_msg,rc,rc_info,p_buf,len,ack涉及宏，慎重修改*/
    unsigned int      rc;
    tf_rc_info       rc_info = {.obj_id = OBJ_DRV_FK, .sub_obj_id.olt_id = MSG2SLOT(p_msg)};
    void              *p_buf = NULL;
    unsigned int      len = 0;
    
    TF_NET_PHY_INFO  phy_info;
    //uint8_t           dev_id;
    //uint8_t           port_id;
    //uint16_t          onu_id;
    
    //获取消息基础参数
    if((rc = tf_net_vif2phy(p_msg->vif_info, &phy_info)))
    {
        printf( "%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        goto ack;
    }
    
    //dev_id = TF_NET_PHY_INFO_TO_DEV_ID(phy_info);
    //port_id = TF_NET_PHY_INFO_TO_TF_ID(phy_info);
    //onu_id = TF_NET_PHY_INFO_TO_ONU_ID(phy_info);
    
    switch(p_msg->cmd_info.cmd_id)
    {
#if 0    
        case NET_MSG_CMD_TF_ACTIVATE_STANDBY:
            {
                TF_FK_NET_PROCESS_REQ_NO_PARAM_VERIFY();
                rc = fk_if_ni_activate_standby(dev_id, port_id);
                TF_FK_NET_ERR_INFO2RC(OBJ_DRV_TF_NI, port_id, rc);
            }
            break;
            
        case NET_MSG_CMD_ONU_BL_SN_CLEAR:
            {
                TF_FK_NET_PROCESS_REQ_NO_PARAM_VERIFY();
                rc = fk_onu_db_onu_filter_tbl_del_all();
                TF_FK_NET_ERR_INFO2RC(OBJ_MDW_SQL, 0, rc);
            }
            break;

        case NET_MSG_CMD_ONU_BL_SW_SET:
            {
                uint8_t *p_sw;
                TF_FK_NET_PROCESS_REQ_PARAM_STRUCT(uint8_t, p_sw);
                rc = fk_onu_db_onu_fliter_tbl_set_switch(*p_sw);
                TF_FK_NET_ERR_INFO2RC(OBJ_MDW_SQL, 0, rc);
            }
            break;
            
        case NET_MSG_CMD_ONU_ROGUE_SIGNAL_RUN_CHECK:
            {
                TF_FK_NET_PROCESS_REQ_NO_PARAM_VERIFY();
                uint8_t *p_info;
                TF_FK_NET_PROCESS_ACK_PARAM_STRUCT(uint8_t, p_info);
                rc = fk_if_onu_rogue_detect(dev_id, port_id, p_info);
                TF_FK_NET_ERR_INFO2RC(OBJ_DRV_TF_NI, port_id, rc);
            }
            break;

        case NET_MSG_CMD_DIGITMAP_PROFILE_SYNC_DELETE:
           {
                uint16_t *p_profile_id;
                TF_FK_NET_PROCESS_REQ_PARAM_STRUCT(uint16_t, p_profile_id);
                rc = fk_if_digitmap_profile_delete_by_id(*p_profile_id);
                TF_FK_NET_ERR_INFO2RC(OBJ_DRV_TF_OLT, 0, rc);
           }
           break;  
#endif           
           
#if 0
        case NET_MSG_CMD_MSG_PC_TEST_SW:
            {
                uint8_t *p_sw;
                TF_FK_NET_PROCESS_REQ_PARAM_STRUCT(uint8_t, p_sw);
                g_fk_packet_test = *p_sw;
               
                TF_FK_NET_ERR_INFO2RC(OBJ_DRV_TF_OLT, 0, rc);
            }
            break;
#endif

        default:
            TF_TF_ERRNO_INFO2RC(rc_info, rc, FK_ZK_RC_MSG_CMD_UNKNOWN);
            break;
    }

    //no error return 0
    if (!(rc%TF_RETURNCODE_BASE))rc = 0;
    
ack:    
    if (rc)
    {
        printf( "%s %s %d msg %d error %d \r\n", __FILE__, __FUNCTION__, __LINE__, p_msg->cmd_info.cmd_id, rc%TF_RETURNCODE_BASE);
    }

    fk_net_msg_send(p_msg, p_buf, len, rc, NET_MSG_DRICTION_ACK);
    tf_net_safe_free(p_buf);
   
    return rc;
}


/*
返回码涉及
1. net -> vif phy info error
2. olt -> olt operation error
3. onu -> table info
4. sql -> operation
*/
static unsigned int 
fk_net_syn_req_olt_get_process(TF_NET_MSG *p_msg)
{
    /*参数名称和标志p_msg,rc,rc_info,p_buf,len,ack涉及宏，慎重修改*/
    unsigned int        rc;
    tf_rc_info         rc_info = {.obj_id = OBJ_DRV_FK, .sub_obj_id.olt_id = MSG2SLOT(p_msg)};
    void                *p_buf = NULL;
    unsigned int        len = 0;
    
    TF_NET_PHY_INFO    phy_info;
    //uint8_t             dev_id;
    //uint8_t             port_id;
    //uint16_t            onu_id;

    //获取消息基础参数
    if ((rc = tf_net_vif2phy(p_msg->vif_info, &phy_info)))
    {
        goto ack;
    }

    //dev_id = TF_NET_PHY_INFO_TO_DEV_ID(phy_info);
    //port_id = TF_NET_PHY_INFO_TO_TF_ID(phy_info);
    //onu_id = TF_NET_PHY_INFO_TO_ONU_ID(phy_info);

    switch(p_msg->cmd_info.cmd_id)
    {
            
#if 0
        //for profile sync
        case NET_MSG_CMD_POTS_PROFILE_DATA_GET:
            {
                uint16_t *p_profile_id;
                TF_FK_NET_PROCESS_REQ_PARAM_STRUCT(uint16_t, p_profile_id);
                POTS_DATA_T *p_data;
                TF_FK_NET_PROCESS_ACK_PARAM_STRUCT(POTS_DATA_T, p_data);
                rc = fk_if_pots_profile_get(*p_profile_id, p_data);
                TF_FK_NET_ERR_INFO2RC(OBJ_DRV_TF_OLT, 0, rc);
            }
        break;
#endif

        default:
            TF_TF_ERRNO_INFO2RC(rc_info, rc, FK_ZK_RC_MSG_CMD_UNKNOWN);
            break;
    }

    //no error return 0
    if (!(rc%TF_RETURNCODE_BASE))rc = 0;

ack:    
    if (rc)
    {
        printf( "%s %s %d msg %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, p_msg->cmd_info.cmd_id, rc%TF_RETURNCODE_BASE);
    }
    
    fk_net_msg_send(p_msg, p_buf, len, rc, NET_MSG_DRICTION_ACK);
    tf_net_safe_free(p_buf);

    return rc;
}

#endif

#if DEFUNC("控制板接收异步响应消息处理接口")

static unsigned int
fk_net_asyn_ack_work_process(TF_NET_MSG *p_msg)
{
    //1.业务板开工
    fk_net_state_set(FK_NET_WORK_SYNCHRONOUS);
    fk_net_state_clear(FK_NET_STATE_WORK_WAITED);
    //2.使能业务板的sn发现处理逻辑

    return 0;
}

//目前业务板没有hb响应，预留接口
static unsigned int
fk_net_asyn_ack_hb_process(TF_NET_MSG *p_msg)
{
    //记录状态
    p_msg = p_msg;
    return 0;
}

static unsigned int
fk_net_asyn_ack_alarm_process(TF_NET_MSG *p_msg)
{
    p_msg = p_msg;
    return 0;
}

#endif
