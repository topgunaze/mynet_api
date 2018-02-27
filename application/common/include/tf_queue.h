/**************************************************************
 * 文件名称:  tf_queue.h
 * 作           者:  keith.gong
 * 日           期:  2014.12.27
 * 文件描述:  系统消息队列管理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

 
#ifndef __TF_QUEUE_H__
#define __TF_QUEUE_H__

#include <tf_types.h>


#define SYS_QUEUE_NO_WAIT                   (0)
#define SYS_QUEUE_WAIT_FOREVER              (-1)


#define SYS_QUEUE_MSG_ANY                   0



typedef struct
{
    //UINT8        *p_packet;  /* pointer to data buffer */
    //void        (*fptr_tx_complete) (UINT8 *p_packet); /*p_packet释放回调函数*/
    //UINT16      size_of_packet; /* length of buffer */
    //INT16       TCI_of_the_packet;/*TCI*/
    //INT16       onu_no;/*onu 序号*/
    UINT8		sub_type;
    UINT32      tfId;
    UINT32      onuUnit;
    UINT32      cmd;
    UINT32      *var;
    INT8        retransmission_timer; /*重发定时器， 单位 秒 */
    INT8        retransmission_count;/*该包已重发的次数*/
}OAMQMsg;

typedef struct
{
    long        type;
    OAMQMsg     msg;
}oam_msg_t;

#define SYS_QUEUE_OAM_PRIORITY_0_SIZE  (1000*sizeof(oam_msg_t))

#define SYS_QUEUE_OAM_PRIORITY_0_RETRA_CNT	3

typedef enum
{
    SYS_QUEUE_OMCI_READY,
    SYS_QUEUE_OMCI_PRIORITY_0,
    SYS_QUEUE_OMCI_PRIORITY_1,

    SYS_QUEUE_TF_CTRL_RX_SYN_REQ_MSG,
    SYS_QUEUE_TF_CTRL_RX_SYN_ACK_MSG,
    SYS_QUEUE_TF_CTRL_RX_ASYN_REQ_MSG,
    SYS_QUEUE_TF_CTRL_RX_ASYN_ACK_MSG,
    SYS_QUEUE_TF_CTRL_TX_MSG,

    SYS_QUEUE_LINEMGMT_MSG,

    SYS_QUEUE_FIR_ONU_CFG_MSG,
    SYS_QUEUE_SEC_ONU_CFG_MSG,
    SYS_QUEUE_ONU_CFG_PROC_NOTIFY_MSG,

    SYS_QUEUE_OAM_PRIORITY_0,

    SYS_QUEUE_MAX_NUM,
}SYS_QUEUE_SUB_KEY_E;

extern RTN_STATUS 
tf_queue_create (
                INT32 *p_queue_id, 
                SYS_QUEUE_SUB_KEY_E sub_key, 
                UINT32 queue_size);

extern RTN_STATUS
tf_queue_get (
                INT32  queue_id, 
                long   type, 
                void   *p_data, 
                UINT32 size, 
                INT32  timeout, /* ms  */
                UINT32 *p_size_copied);
extern RTN_STATUS
tf_queue_put (
                INT32  queue_id, 
                void   *p_data, 
                UINT32 size, 
                INT32  timeout);


#endif

