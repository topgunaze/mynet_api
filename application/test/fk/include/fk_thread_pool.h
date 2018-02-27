/**************************************************************
* 文件名称:  
* 作            者: 
* 日            期:  
* 文件描述:
* 版            本:  
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>
**************************************************************/

#ifndef __TF_THREAD_POOL__
#define __TF_THREAD_POOL__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

#include "tf_adaptor_common_if.h"

#define MAX_NODE_EACH_PORT           128
#define MAX_NUM_OF_PORT              16

#define INVALID_KEY                     0xFFFFFFFEU  // 0xFFFFFFFFU -> 0xFFFFFFFEU 确保与INVALID_PHY_UINT 等key的非法值不一样
#define INVALID_PHYUINT_CONVERSION_KEY  2048
#define MAX_WORKER_GROUP                (1)
#define MAX_WORKER_NUM                  (MAX_WORKER_GROUP*MAX_NODE_EACH_PORT)
#define WORKER_THREAD_LIMIT_UPGRADE     14

#define KEY_PHYUINT2KEY(phy_uint)       do{if(phy_uint == INVALID_PHY_UINT)phy_uint = INVALID_PHYUINT_CONVERSION_KEY;}while(0)
#define KEY2PHYUINT_KEY(key)            do{if(key == INVALID_PHYUINT_CONVERSION_KEY)key = INVALID_PHY_UINT;}while(0)

typedef enum
{
    WORKER_TYPE_COMMON,
    WORKER_TYPE_UPGRADE,

    MAX_WORKER_TYPE_NUM,
}
THREAD_WORKER_TYPE;

typedef void (*THREAD_WORKER_CB)(const uint32_t key, const uint32_t arg1);
typedef void (*THREAD_WORKER_RELEASE_CB)(void);


//节点设备任务节点
typedef struct TF_NODE_WORKER
{
    STAILQ_ENTRY(TF_NODE_WORKER)   next;

    THREAD_WORKER_CB                handler;
    uint32_t                        arg1;
    uint8_t                         worker_type;
    uint8_t                         remove_flag;
    uint8_t                         rsvd[2];    /*用于字节对齐*/
} TF_NODE_WORKER;

//节点设备任务链表
typedef STAILQ_HEAD(, TF_NODE_WORKER) TF_NODE_WORKER_LIST;

typedef struct
{
    THREAD_WORKER_RELEASE_CB    release_cb;
    uint16_t                    thread_cnt;
    uint16_t                    thread_limit;
    uint32_t                    worker_cnt;
} THREAD_WORKER_TYPE_INFO;

typedef struct
{
    TF_NODE_WORKER_LIST    *p_node_worker_list;

    pthread_mutex_t         worker_list_lock;
    pthread_cond_t          worker_list_cond;

    uint16_t                max_thread_num;
    pthread_t               *p_thread_id_queue;
    uint32_t                *p_processing_key_queue;
    THREAD_WORKER_TYPE_INFO type_info[MAX_WORKER_TYPE_NUM];
    uint8_t                 worker_bitmap[MAX_WORKER_GROUP][MAX_NODE_EACH_PORT/8];

    uint8_t                 shutdown_flag;

} THREAD_POOL;

int
tf_thread_pool_worker_type_set_release_cb (
                THREAD_POOL                 *p_thread_pool,
                const uint8_t               worker_type,
                THREAD_WORKER_RELEASE_CB    release_cb);


int
tf_thread_pool_worker_type_set_limit (
                THREAD_POOL         *p_thread_pool,
                const uint8_t       worker_type,
                const uint16_t      limit);

int
tf_thread_pool_add_worker (
                THREAD_POOL             *p_thread_pool,
                const THREAD_WORKER_CB  handler,
                const uint8_t           worker_type,
                uint32_t                key,
                const uint32_t          arg1);

int
tf_thread_pool_clear_worker (
                THREAD_POOL     *p_thread_pool,
                uint32_t        key,
                const uint8_t   work_type);

int
tf_thread_pool_destory(
                THREAD_POOL *p_thread_pool);

void
fk_thread_pool_init (
                THREAD_POOL     *p_thread_pool,
                const uint16_t  max_thread_num);

#endif /* __TF_THREAD_POOL__ */

