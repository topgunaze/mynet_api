/**************************************************************
* 文件名称: 
* 文件描述:
* 版           本: 
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>

**************************************************************/

#ifndef __FK_NET__
#define __FK_NET__

#include "tf_adaptor_net_if.h"

typedef enum
{
    FK_DEVICE_STATE_INIT           = 0,
    FK_NET_CONNECTION_COMPLETED    = 1,//建立与控制面连接
    FK_DEVICE_STATE_READY          = 2,//建立与68620的连接
    FK_DEVICE_STATE_STANDBY        = 3,//待命
    FK_NET_WORK_SYNCHRONOUS        = 4,//控制面控制开工
    FK_NET_STATE_WORKING           = 7,//工作
    FK_NET_STATE_WORK_WAITED       = 8,//等待开工

    FK_STATE_NUM_OF
}TF_FK_NET_STATE;

typedef struct
{
    TF_NET_MUTUX     mutex;
    TF_NET_PARA      net_param;   /*业务板连接信息*/
    TF_FK_NET_STATE  state;       /*业务板网络状态*/
    uint8_t          slot_id;     /*业务板槽位信息*/

}TF_FK_STATE;

//业务板板间消息并发处理线程数，
#define FK_NET_MSG_CC_PRO_NUM   9
#define FK_NET_MSG_SER_PRO_ID   8
#define FK_NET_MSG_CC_PRO_MASK  0x07

//请求消息处理错误返回码
//没有传参
#define TF_FK_NET_PROCESS_REQ_NO_PARAM_VERIFY()               TF_TF_NET_PROCESS_REQ_NO_PARAM_STRUCT(p_msg,rc_info,rc,FK_ZK_RC_PARAM_OUT_OF_RANGE,ack)
//有传参
#define TF_FK_NET_PROCESS_REQ_PARAM_STRUCT(type,p_name)       TF_TF_NET_PROCESS_REQ_PARAM_STRUCT(type,p_name,p_msg,rc_info,rc,FK_ZK_RC_PARAM_OUT_OF_RANGE,ack)
//有回传参数
#define TF_FK_NET_PROCESS_ACK_PARAM_STRUCT(type,p_name)       TF_TF_NET_PROCESS_ACK_PARAM_STRUCT(type,p_name,g_fk_net_msg_mp,p_buf,len,rc_info,rc,FK_ZK_RC_MEM_ALLOCATION,ack)

#define TF_FK_NET_ERR_INFO2RC(obj,sub_obj,err_no)             TF_TF_ERR_INFO2RC(rc_info,obj,sub_obj,err_no,rc)


unsigned int fk_net_ev_init(void);
unsigned int fk_net_task_init(void);

unsigned int 
fk_net_mempool_init(void);

uint32_t
fk_net_syn_req_profile_get(const uint8_t slot_id,
                                MSG_PC_TEST *p_in,
                                MSG_PC_TEST *p_out);

uint32_t 
fk_net_syn_operation(  IN const TF_NET_MSG_TYPE  msg_type,
                                IN const  uint64_t         vif_info, 
                                IN const  TF_NET_MSG_CMD  cmd_info, 
                                IN const  void             *pvar, 
                                IN const  uint32_t         pvar_len,
                                OUT void                   *param,
                                IN  const uint32_t         param_len,
                                OUT uint32_t               *state);

uint32_t 
fk_net_msg_syn_ack_process(TF_NET_MSG *pmsg);

void
fk_net_state_set(int state);

uint32_t
fk_net_state_get(void);

void
fk_net_state_clear(int state);

uint32_t
fk_net_state_check(const int state);

uint32_t
fk_net_state_prepare_work(void);

uint32_t
fk_net_state_is_work(void);

#endif
