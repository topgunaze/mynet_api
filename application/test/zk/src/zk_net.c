/**************************************************************
 * 文件名称:  
 * 作           者: 
 * 日           期:  
 * 文件描述:  
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
 
**************************************************************/

#include "ev.h"
#include "zk_net.h"
//#include "gtfAlarm.h"
#include <sys/sysinfo.h>
//#include "tf_bit_map.h"

//for packet test
#if 0
INT32 g_tf_ctrl_msg_rec_count;
INT32 g_tf_ctrl_msg_rec_hb_load_count;
extern char g_msg_pc_test_buf[MSG_BODY_LEN_TEST_PC];
#endif

static void
tf_ctrl_net_active_disconnect_clear(EV_P_ struct ev_io *watcher);
  
//接收消息(开工、心跳、自发、告警自发)异步处理接口
static UINT32 
tf_ctrl_net_asyn_req_work_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_req_hb_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_req_auto_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_alarm_asyn_req_alarm_process(TF_NET_MSG *pmsg);

//接收响应消息(onu/olt/nw配置与配置获取)的异步处理预留接口
static UINT32 
tf_ctrl_net_asyn_ack_onu_set_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_ack_onu_get_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_ack_olt_set_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_ack_olt_get_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_ack_nw_set_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_asyn_ack_nw_get_process(TF_NET_MSG *pmsg);

static UINT32 
tf_ctrl_net_syn_req_drv_pull_process(TF_NET_MSG *pmsg);

UINT32  g_tf_ctrl_slot[TF_NET_MANAGE_MAX_SLOT];

//ctrl 消息队列消息内存池
static TF_NET_BLK_POOL g_tf_ctrl_tr_mq_mp;
//控制板间消息内存池
static TF_NET_BLK_POOL g_tf_ctrl_net_msg_mp;
//控制板libev 监视器内存池
static TF_NET_BLK_POOL g_tf_ctrl_watcher_mp;
//控制板同步消息连接内存池
static TF_NET_BLK_POOL g_tf_ctrl_conn_mp;
//控制板同步消息内存池
static TF_NET_BLK_POOL g_tf_ctrl_conn_msg_mp;

//控制板发送消息队列
static TF_NET_ZC_MQ g_tf_ctrl_tx_mq[TF_NET_MANAGE_MAX_SLOT];
//控制板接收同步请求消息队列
static TF_NET_ZC_MQ g_tf_ctrl_rx_syn_req_mq[TF_NET_MANAGE_MAX_SLOT];
//控制板接收同步响应消息队列
static TF_NET_ZC_MQ g_tf_ctrl_rx_syn_ack_mq[TF_NET_MANAGE_MAX_SLOT];
//控制板接收异步请求消息队列
static TF_NET_ZC_MQ g_tf_ctrl_rx_asyn_req_mq;
//控制板接收异步响应消息队列
static TF_NET_ZC_MQ g_tf_ctrl_rx_asyn_ack_mq;

//连接映射
static TF_TF_CTRL_NET_MAP     g_tf_ctrl_net_map[TF_NET_MANAGE_MAX_SLOT];
//控制板状态
static TF_TF_CTRL_STATE       g_tf_ctrl_state[TF_NET_MANAGE_MAX_SLOT];
//控制板全局表项配置状态
TF_TF_CTRL_GBL_CFG_STATE      g_tf_ctrl_gbl_cfg_state;
//conn 公共锁
static TF_NET_MUTUX            g_tf_ctrl_conn_lock;
//控制板与业务板之间的同步消息连接
static TF_NET_CONN             *g_tf_ctrl_conn[TF_NET_MANAGE_MAX_SLOT];

//该宏只在本文件使用
#define SLOT2NET_IO(slot_id)        (g_tf_ctrl_net_map[g_tf_ctrl_state[slot_id].map_id].net_param)
#define SLOT2NET_FD(slot_id)        (g_tf_ctrl_net_map[g_tf_ctrl_state[slot_id].map_id].net_param.watcher->fd)

#if DEFUNC("控制板全局配置同步接口")

//网络地址映射初始化
void
tf_ctrl_net_cfg_init(void)
{ 
    bzero(&g_tf_ctrl_gbl_cfg_state, sizeof(TF_TF_CTRL_GBL_CFG_STATE));
    tf_net_mutex_create(&g_tf_ctrl_gbl_cfg_state.mutex);
    tf_net_mutex_create(&g_tf_ctrl_gbl_cfg_state.sync_lock);

    return ;
}

//全局配置项有效标示设置
unsigned int
tf_ctrl_net_cfg_valid_set(
                            const uint8_t   type_idx,
                            const uint16_t  item_idx)
{  
    if (type_idx >= TF_NET_CFG_TYPE_NUM_OF || item_idx >= TF_NET_CFG_ITEM_NUM)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }
    
    tf_net_mutex_lock(&g_tf_ctrl_gbl_cfg_state.mutex);
    //bitListSet(g_tf_ctrl_gbl_cfg_state.gbl_cfg[type_idx].valid, item_idx);
    tf_net_mutex_unlock(&g_tf_ctrl_gbl_cfg_state.mutex);

    return 0;
}

//全局配置项有效标示清除
unsigned int
tf_ctrl_net_cfg_valid_clear(
                            const uint8_t   type_idx,
                            const uint16_t  item_idx)
{  
    if (type_idx >= TF_NET_CFG_TYPE_NUM_OF || item_idx >= TF_NET_CFG_ITEM_NUM)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }
    
    tf_net_mutex_lock(&g_tf_ctrl_gbl_cfg_state.mutex);
    //bitListClr(g_tf_ctrl_gbl_cfg_state.gbl_cfg[type_idx].valid, item_idx);
    tf_net_mutex_unlock(&g_tf_ctrl_gbl_cfg_state.mutex);

    return 0;
}

//全局配置项是否有效
unsigned int
tf_ctrl_net_cfg_is_valid(
                            const uint8_t   type_idx,
                            const uint16_t  item_idx)
{  
    if (type_idx >= TF_NET_CFG_TYPE_NUM_OF || item_idx >= TF_NET_CFG_ITEM_NUM)
    {
        return 0;
    }

    return 0;
    //return bitListTst(g_tf_ctrl_gbl_cfg_state.gbl_cfg[type_idx].valid, item_idx);
}

//全局配置板间同步状态
void
tf_ctrl_net_cfg_sync_lock(void)
{   
    tf_net_mutex_lock(&g_tf_ctrl_gbl_cfg_state.sync_lock);

    return ;     
}

//全局配置正在板间同步
uint32_t
tf_ctrl_net_cfg_sync_trylock(void)
{    
   return tf_net_mutex_trylock(&g_tf_ctrl_gbl_cfg_state.sync_lock);   
}

//全局配置板非板间同步状态
void
tf_ctrl_net_cfg_sync_unlock(void)
{   
    tf_net_mutex_unlock(&g_tf_ctrl_gbl_cfg_state.sync_lock);

    return ;     
}
    
#endif

#if DEFUNC("控制板网络映射接口")

//网络地址映射初始化
void
tf_ctrl_net_map_init(void)
{
    int idx;
    
    for (idx = 0; idx < TF_NET_MANAGE_MAX_SLOT; idx++)
    {
        bzero(&g_tf_ctrl_net_map[idx], sizeof(TF_TF_CTRL_NET_MAP));
        tf_net_mutex_create(&g_tf_ctrl_net_map[idx].mutex);
        g_tf_ctrl_net_map[idx].slot_id = NET_COM_PARAM_INVALID;
    }

    return ;
}

//网络地址映射占用状态设置
static unsigned int
tf_ctrl_net_map_used_set(
                            const uint8_t idx,
                            const uint8_t is_used)
{  
    if (idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }
    
    tf_net_mutex_lock(&g_tf_ctrl_net_map[idx].mutex);
    g_tf_ctrl_net_map[idx].is_used = is_used;
    tf_net_mutex_unlock(&g_tf_ctrl_net_map[idx].mutex);

    return 0;
}

//网络地址映射slot
static unsigned int
tf_ctrl_net_map_slot_set(
                            const uint8_t idx,
                            const uint8_t slot_id)
{  
    if (idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }

    tf_net_mutex_lock(&g_tf_ctrl_net_map[idx].mutex);
    g_tf_ctrl_net_map[idx].slot_id = slot_id;
    tf_net_mutex_unlock(&g_tf_ctrl_net_map[idx].mutex);

    return 0;
}

//网络地址映射时间戳设置
static unsigned int
tf_ctrl_net_map_time_stamp_set(
                            const uint8_t  idx,
                            const uint32_t time)
{  
    if (idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }

    tf_net_mutex_lock(&g_tf_ctrl_net_map[idx].mutex);
    g_tf_ctrl_net_map[idx].timeout_stamp = time;
    tf_net_mutex_unlock(&g_tf_ctrl_net_map[idx].mutex);

    return 0;
}

//网络地址映射地址设置
static unsigned int
tf_ctrl_net_map_net_param_set(
                            uint8_t             idx,
                            const TF_NET_PARA  *p_param)
{  
    if (idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }

    tf_net_mutex_lock(&g_tf_ctrl_net_map[idx].mutex);
    memcpy(&g_tf_ctrl_net_map[idx].net_param, p_param, sizeof(TF_NET_PARA));
    tf_net_mutex_unlock(&g_tf_ctrl_net_map[idx].mutex);

    return 0;
}

//网络地址映射空闲记录获取
static uint32_t
tf_ctrl_net_map_free_id_get(uint32_t *p_idx)
{
    int idx;

    if (!p_idx)
    {
        return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    //记录板间连接fd
    for (idx = 0; idx < TF_NET_MANAGE_MAX_SLOT; idx++)
    {
        if (g_tf_ctrl_net_map[idx].is_used)
            continue;

        break;
    }

    if (idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_NET_MAP_IS_FULL;
    }

    *p_idx = idx;
    tf_ctrl_net_map_used_set(idx, NET_COM_PARAM_IS_SET);

    return 0;
}

//网络地址映射记录设置
static unsigned int
tf_ctrl_net_map_net_param_init(
                                    struct ev_loop     *loop,
                                    struct ev_io       *watcher,
                                    uint8_t            map_id, 
                                    struct sockaddr_in *p_addr)
{
    struct sysinfo sys_info;
    TF_NET_PARA    net_param;

    if (map_id >= TF_NET_MANAGE_MAX_SLOT || !p_addr)
    {
        return TF_APP_ZK_RC_PARAM_INVALID;
    }

    net_param.loop    = loop;
    net_param.watcher = watcher;
    memcpy(&net_param.remote_addr, p_addr, sizeof(struct sockaddr_in));
    
    tf_ctrl_net_map_net_param_set(map_id, &net_param);
    
    sysinfo(&sys_info);
    
    tf_ctrl_net_map_time_stamp_set(map_id, sys_info.uptime);
    
    printf("accept time out statmp %ld\r\n", sys_info.uptime + TF_CTRL_STATE_FD_TIMEOUT);

    return 0;
}

#endif

#if DEFUNC("控制板工作状态接口")

//控制板板间网络状态初始化
void
tf_ctrl_net_state_init(void)
{
    int slot_id;
    
    for (slot_id = 0; slot_id < TF_NET_MANAGE_MAX_SLOT; slot_id++)
    {
        g_tf_ctrl_slot[slot_id] = slot_id;
        bzero(&g_tf_ctrl_state[slot_id], sizeof(TF_TF_CTRL_NET_STATE));
        g_tf_ctrl_state[slot_id].map_id = NET_COM_PARAM_INVALID; 
        tf_net_mutex_create(&g_tf_ctrl_state[slot_id].mutex);
    }

    return ;
}

//控制板板间网路状态设置
static unsigned int
tf_ctrl_net_state_set(
                            const uint8_t   slot_id,
                            const uint32_t  state)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    g_tf_ctrl_state[slot_id].state |= state;
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间网路状态清除
static unsigned int
tf_ctrl_net_state_clear(
                            const uint8_t    slot_id,
                            const uint32_t   state)
{     
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    g_tf_ctrl_state[slot_id].state &= ~state;
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间网路状态已开工
uint32_t
tf_ctrl_net_state_working(const uint8_t slot_id)
{   
    uint32_t state;
    
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return 0;
        
   state = g_tf_ctrl_state[slot_id].state & TF_CTRL_NET_WORK_SYNCHRONOUS;
   
   return (state == TF_CTRL_NET_WORK_SYNCHRONOUS) ?  1 : 0;     
}
/*
//控制板板间网路tf口在线onu递增
static unsigned int
tf_ctrl_net_port_onu_online_add(const uint8_t slot_id, const uint8_t port_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || port_id >= SYS_MAX_EXIST_PORT_NUM)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
        
    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    ++g_tf_ctrl_state[slot_id].online_num[port_id];
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}
*/
//控制板板间网路tf口在线onu数目获取
uint8_t
tf_ctrl_net_port_onu_online_get(
                            const uint8_t slot_id,
                            const uint8_t port_id)
{    
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || port_id >= SYS_MAX_EXIST_PORT_NUM)
        return 0;
        
    return g_tf_ctrl_state[slot_id].online_num[port_id];
}
/*
//控制板板间网路tf口在线onu数目递减
static unsigned int
tf_ctrl_net_port_onu_online_dec(const uint8_t slot_id, const uint8_t port_id)
{
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || port_id >= SYS_MAX_EXIST_PORT_NUM)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    --g_tf_ctrl_state[slot_id].online_num[port_id];
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);
    
    return 0;
}
*/
//控制板板间网路心跳统计设置
static unsigned int
tf_ctrl_net_hb_cnt_set(
                            const uint8_t slot_id,
                            const uint8_t cnt)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    g_tf_ctrl_state[slot_id].hb_timeout_cnt = cnt;
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间网路心跳统计递增
static unsigned int
tf_ctrl_net_hb_cnt_add(const uint8_t slot_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    ++g_tf_ctrl_state[slot_id].hb_timeout_cnt;
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间网路心跳超时统计获取
static uint8_t
tf_ctrl_net_hb_cnt_get(const uint8_t slot_id)
{    
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return NET_COM_PARAM_INVALID;

    return g_tf_ctrl_state[slot_id].hb_timeout_cnt;
}

//控制板板间网络映射设置
static unsigned int
tf_ctrl_net_map_id_set(const uint8_t slot_id, const uint8_t map_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
        
    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    g_tf_ctrl_state[slot_id].map_id = map_id;
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间全局配置完成标示设置
unsigned int
tf_ctrl_net_cfg_sync_valid_set(
                            const uint8_t   slot_id,
                            const uint8_t   type_id,
                            const uint16_t  item_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || type_id >= TF_NET_CFG_TYPE_NUM_OF
        || item_id >= TF_NET_CFG_ITEM_NUM)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    //bitListSet(g_tf_ctrl_state[slot_id].cfg_sync[type_id].valid, item_id);
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间全局配置完成标示清除
unsigned int
tf_ctrl_net_cfg_sync_valid_clear(
                            const uint8_t   slot_id,
                            const uint8_t   type_id,
                            const uint16_t  item_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || type_id >= TF_NET_CFG_TYPE_NUM_OF
        || item_id >= TF_NET_CFG_ITEM_NUM)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;

    tf_net_mutex_lock(&g_tf_ctrl_state[slot_id].mutex);
    //bitListClr(g_tf_ctrl_state[slot_id].cfg_sync[type_id].valid, item_id);
    tf_net_mutex_unlock(&g_tf_ctrl_state[slot_id].mutex);

    return 0;
}

//控制板板间全局配置是否完成
unsigned int
tf_ctrl_net_cfg_is_sync_valid(                            
                            const uint8_t   slot_id,
                            const uint8_t   type_id,
                            const uint16_t  item_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT || type_id >= TF_NET_CFG_TYPE_NUM_OF
        || item_id >= TF_NET_CFG_ITEM_NUM)
        return 0;
    return 0;    
    //return bitListTst(g_tf_ctrl_state[slot_id].cfg_sync[type_id].valid, item_id);
}

//控制板板间全局配置是否完成
void
tf_ctrl_net_cfg_sync_invalid_set(const uint8_t slot_id)
{  
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return ;
        
    bzero(g_tf_ctrl_state[slot_id].cfg_sync, TF_CFG_SYNC_DATA_TOTAL_LEN);
}

#endif

#if DEFUNC("全局配置比对接口")

//全局配置项比对
unsigned int 
tf_ctrl_net_cfg_is_consistent(const uint8_t slot_idx)
{
    //非法入参不涉及比对
    if (slot_idx >= TF_NET_MANAGE_MAX_SLOT)
    {
        return 1;
    }
    
    return !memcmp(g_tf_ctrl_state[slot_idx].cfg_sync, g_tf_ctrl_gbl_cfg_state.gbl_cfg, TF_CFG_SYNC_DATA_TOTAL_LEN);
}

//类型配置比对
unsigned int
tf_ctrl_net_cfg_type_is_consistent(
                                        const uint8_t slot_idx, 
                                        const uint8_t type_idx)
{
    //非法入参不涉及比对
    if (slot_idx >= TF_NET_MANAGE_MAX_SLOT || type_idx >= TF_NET_CFG_TYPE_NUM_OF)
    {
        return 1;
    }

    return !memcmp(&g_tf_ctrl_state[slot_idx].cfg_sync[type_idx], &g_tf_ctrl_gbl_cfg_state.gbl_cfg[type_idx], TF_CFG_SYNC_DATA_LEN);
}

//类型配置项组比对
unsigned int
tf_ctrl_net_cfg_item_group_is_consistent(
                                        const uint8_t slot_idx, 
                                        const uint8_t type_idx, 
                                        const uint8_t group_idx)
{
    //非法入参不涉及比对
    if (slot_idx >= TF_NET_MANAGE_MAX_SLOT || type_idx >= TF_NET_CFG_TYPE_NUM_OF
        || group_idx >= TF_NET_CFG_ITEM_NUM/8)
    {
        return 1;
    }

    return !memcmp(&g_tf_ctrl_state[slot_idx].cfg_sync[type_idx].valid[group_idx], &g_tf_ctrl_gbl_cfg_state.gbl_cfg[type_idx].valid[group_idx], sizeof(uint8_t));
}

#endif

#if DEFUNC("控制板网络建链处理接口")

//未完成建立连接的连接信息
static uint32_t
tf_ctrl_net_map_id_get_by_connect(const uint32_t fd, uint8_t *p_map_id)
{
    uint8_t idx;

    for (idx = 0; idx < TF_NET_MANAGE_MAX_SLOT; idx++)
    {
        if (g_tf_ctrl_net_map[idx].net_param.watcher->fd == fd)
        {
            *p_map_id = idx;
            //fd match
            return 0;
        }
    }

    //not match fd, no record impossibility!!!
    return TF_APP_ZK_RC_IMPOSSIBLE_ERROR; 
}

//已经完成认证的连接信息
static uint32_t
tf_ctrl_net_map_info_get_by_connect(
                                    const uint32_t  fd, 
                                    uint8_t         *p_slot_id, 
                                    uint8_t         *p_map_id)
{
    uint8_t idx;

    if (!p_slot_id || !p_map_id)
    {
        return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    for (idx = 0; idx < TF_NET_MANAGE_MAX_SLOT; idx++)
    {
        if (g_tf_ctrl_net_map[idx].net_param.watcher->fd == fd)
        {
            //fd match, slot info
            *p_map_id  = idx;
            *p_slot_id = g_tf_ctrl_net_map[idx].slot_id;
            return 0;
        }
    }
    
    //not match fd, no record impossibility!!!
    return TF_APP_ZK_RC_IMPOSSIBLE_ERROR;
}

static uint32_t
tf_ctrl_net_map_conflict_check(const uint8_t slot_id)
{
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
        
    if (g_tf_ctrl_state[slot_id].map_id != NET_COM_PARAM_INVALID)
        return TF_APP_ZK_RC_NET_MAP_ALREADY_EXIST;
        
    return 0;
}

static uint32_t
tf_ctrl_net_map_process(const uint8_t map_id, const uint8_t slot_id)
{
    uint32_t rc;

    //check slot info conflict
    if ((rc = tf_ctrl_net_map_conflict_check(slot_id)))
    {
        return rc;
    }
    
    tf_ctrl_net_map_id_set(slot_id, map_id);
    tf_ctrl_net_map_slot_set(map_id, slot_id);
    
    return 0;
}

static uint32_t
tf_ctrl_net_work_synch_process(const TF_NET_MSG *pmsg, const uint8_t slot_id)
{
    uint32_t rc;
    uint8_t  map_id;

    if(!pmsg)
    {
        rc = TF_APP_ZK_RC_PARAM_INVALID;
        goto syn_fail;
    }

    tf_ctrl_net_map_id_get_by_connect(pmsg->fd, &map_id);

    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
    {
        rc = TF_APP_ZK_RC_PARAM_INVALID;
        goto syn_fail;
    }
      
    //完成slot与链路之间的映射,slot的校验在映射中完成
    if ((rc = tf_ctrl_net_map_process(map_id, slot_id)))
    {
        goto syn_fail;
    }

    //同步板间链路状态
    tf_ctrl_net_state_set(slot_id, TF_CTRL_NET_WORK_SYNCHRONOUS);
    
    printf("tf ctrl rec req work msg net map syn success, the conn slot %d, fd %d\r\n", slot_id, pmsg->fd);

    return rc;
    
syn_fail:

    //断开当前连接由事件监听处清理连接资源
    if(pmsg)
    {
        close(pmsg->fd);
        tf_ctrl_net_active_disconnect_clear(g_tf_ctrl_net_map[map_id].net_param.loop, 
                                            g_tf_ctrl_net_map[map_id].net_param.watcher);
    }
    
    printf("tf ctrl rec req work msg net syn map fail reason %d\r\n", rc);
    
    return rc;
}

#endif

#if DEFUNC("控制板网络断链处理接口")

static void
tf_ctrl_net_active_disconnect_clear(EV_P_ struct ev_io *watcher)
{
    uint8_t map_id;
    
    //已释放tcp连接资源
    tf_ctrl_net_map_id_get_by_connect(watcher->fd, &map_id);
    //1. 释放连接映射资源
    tf_ctrl_net_map_used_set(map_id, 0);
    //2. 释放连接对应的读事件
    ev_io_stop(loop, watcher);
    tf_net_safe_free(watcher);

    return ;
}

static void
tf_ctrl_net_inactive_disconnect_clear(EV_P_ struct ev_io *watcher)
{
    uint8_t slot_id, map_id;

    //1. 清空对应slot的网络状态  
    tf_ctrl_net_map_info_get_by_connect(watcher->fd, &slot_id, &map_id);
    tf_ctrl_net_state_clear(slot_id, TF_CTRL_NET_WORK_SYNCHRONOUS);
    tf_ctrl_net_cfg_sync_invalid_set(slot_id);
    //2. 释放连接映射资源
    tf_ctrl_net_map_used_set(map_id, NET_COM_PARAM_INIT);
    tf_ctrl_net_map_slot_set(map_id, NET_COM_PARAM_INVALID);
    tf_ctrl_net_map_id_set(slot_id,  NET_COM_PARAM_INVALID);
    //3. 释放tcp连接资源
    close(watcher->fd);
    //4. 释放连接对应的读事件
    ev_io_stop(loop,watcher);
    tf_net_safe_free(watcher);

    return ;
}

#endif

#if DEFUNC("控制板定时事件处理接口")

static void 
tf_ctrl_net_hb_timeout_process(uint8_t idx)
{
    //对开工的TF卡开启心跳监听    
    if (tf_ctrl_net_state_working(idx))
    {
        tf_ctrl_net_hb_cnt_add(idx);

        if (tf_ctrl_net_hb_cnt_get(idx) > TF_CTRL_STATE_HB_LOSE_CNT)
        {
            //告警
            printf("tf ctrl hb check timeout slot %d!!!\n", idx);
        }
    }

    return ;
}

static void 
tf_ctrl_net_map_timeout_process(uint8_t idx)
{
    struct sysinfo sys_info;
    uint32_t       duration;

    //检测占用连接超时
    if (g_tf_ctrl_net_map[idx].is_used && 
        g_tf_ctrl_net_map[idx].slot_id == NET_COM_PARAM_INVALID)
    {
        sysinfo(&sys_info);
        
        if (g_tf_ctrl_net_map[idx].timeout_stamp <= sys_info.uptime)
        {
            duration = sys_info.uptime - g_tf_ctrl_net_map[idx].timeout_stamp;
        }
        else
        {
            //in case not possible 
            duration = TF_CTRL_STATE_FD_TIMEOUT;
        }
  
        if (duration >= TF_CTRL_STATE_FD_TIMEOUT)
        {
            //连接超时需要清除
            printf("tf ctrl net map timeout !!!\r\n"
                   "current time %ld stamp time %u\r\nconn fd %d addr %s port %d\r\n", 
                    sys_info.uptime,
                    g_tf_ctrl_net_map[idx].timeout_stamp,
                    g_tf_ctrl_net_map[idx].net_param.watcher->fd,
                    inet_ntoa(g_tf_ctrl_net_map[idx].net_param.remote_addr.sin_addr),
                    ntohs(g_tf_ctrl_net_map[idx].net_param.remote_addr.sin_port));
         
            close(g_tf_ctrl_net_map[idx].net_param.watcher->fd);
            //主动断链
            tf_ctrl_net_active_disconnect_clear(g_tf_ctrl_net_map[idx].net_param.loop, 
                                                g_tf_ctrl_net_map[idx].net_param.watcher);
            
        }
    }

    return ;
}

#endif

#if DEF_FUNC("接收消息异步处理接口")

//异步请求消息处理函数组
static net_msg_handler tf_ctrl_net_asyn_req_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            tf_ctrl_net_asyn_req_work_process,
            tf_ctrl_net_asyn_req_hb_process,
            NULL, NULL, NULL, NULL, NULL, NULL,
            tf_ctrl_net_asyn_req_auto_process,
            tf_ctrl_net_alarm_asyn_req_alarm_process,
            NULL}; 

//异步响应消息处理函数组
static net_msg_handler tf_ctrl_net_asyn_ack_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            NULL, NULL,
            tf_ctrl_net_asyn_ack_onu_set_process,
            tf_ctrl_net_asyn_ack_onu_get_process,
            tf_ctrl_net_asyn_ack_olt_set_process,
            tf_ctrl_net_asyn_ack_olt_get_process,
            tf_ctrl_net_asyn_ack_nw_set_process,
            tf_ctrl_net_asyn_ack_nw_get_process,
            NULL, NULL, NULL}; 

//处理业务板异步请求消息
static UINT32 
tf_ctrl_net_msg_asyn_req_process(TF_NET_MSG *pmsg)
{
    UINT32 rc = 0;

    if (tf_ctrl_net_asyn_req_handler[pmsg->msg_type])
    {
        if ((rc = tf_ctrl_net_asyn_req_handler[pmsg->msg_type](pmsg)))
        {
            printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

    }
    else
    {
        printf("%s %s %d req tf ctrl asyn msg handler not support msg type %d!!! \n", __FILE__, __FUNCTION__, __LINE__, 
                pmsg->msg_type);
    }
    
    return rc;
}

//处理业务板异步响应消息
static UINT32 
tf_ctrl_net_msg_asyn_ack_process(TF_NET_MSG *pmsg)
{
    UINT32 rc = 0;

    if (tf_ctrl_net_asyn_ack_handler[pmsg->msg_type])
    {
        if ((rc = tf_ctrl_net_asyn_ack_handler[pmsg->msg_type](pmsg)))
        {
            printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        }
    }
    else
    {
        printf("%s %s %d ack tf ctrl asyn msg not support msg type %d!!! \n", __FILE__, __FUNCTION__, __LINE__, 
                pmsg->msg_type);
    }
    
    return rc;
}

//接收异步请求消息处理任务
static void 
tf_ctrl_net_rx_asyn_req_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    UINT32          rc;

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_tf_ctrl_rx_asyn_req_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf("%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息
        if ((rc = tf_ctrl_net_msg_asyn_req_process(p_usr_msg)))
        {
            printf("%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//接收同步响应消息处理任务
static void 
tf_ctrl_net_rx_asyn_ack_task(void)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    UINT32          rc;

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(&g_tf_ctrl_rx_asyn_ack_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf("%s %s %d error %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息
        if ((rc = tf_ctrl_net_msg_asyn_ack_process(p_usr_msg)))
        {
            printf("%s %s %d error %d!!! \r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//异步请求消息处理任务初始化
static UINT32 
tf_ctrl_net_rx_asyn_req_task_init()
{
    pthread_t       tid;
    pthread_attr_t  attr;
    UINT32          rc;
    
    //rx queue
    rc = tf_net_zc_mq_create(&g_tf_ctrl_rx_asyn_req_mq, "server_rx_asyn_req_mq");
    if (rc)
    {
        printf("%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return TF_APP_ZK_RC_MSG_QUEUE_CREATE;
    }

    //rx task
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_rx_asyn_req_task, NULL) < 0)
    {
        printf("%s %s %d errorno %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return TF_APP_ZK_RC_TASK_CREATE;
    }

    return 0;
}

//异步响应消息处理任务初始化
static UINT32 
tf_ctrl_net_rx_asyn_ack_task_init()
{
    pthread_t       tid;
    pthread_attr_t  attr;
    UINT32          rc;
    
    //rx queue
    rc = tf_net_zc_mq_create(&g_tf_ctrl_rx_asyn_ack_mq, "server_rx_asyn_ack_mq");
    if (rc)
    {
        printf("%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
        return TF_APP_ZK_RC_MSG_QUEUE_CREATE;
    }

    //rx task
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_rx_asyn_ack_task, NULL) < 0)
    {
        printf("%s %s %d errorno %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return TF_APP_ZK_RC_TASK_CREATE;
    }

    return 0;
}

#endif

#if DEF_FUNC("接收消息同步处理接口")

//处理业务板同步请求消息函数组
static net_msg_handler tf_ctrl_net_req_syn_handler[NET_MSG_TYPE_NUM_OF] = {
            NULL,
            NULL, NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL, NULL,
            tf_ctrl_net_syn_req_drv_pull_process}; 

//处理业务板同步请求消息接口
static UINT32 
tf_ctrl_net_msg_syn_req_process(TF_NET_MSG *pmsg)
{
    UINT32 rc = 0;
    
    if (tf_ctrl_net_req_syn_handler[pmsg->msg_type])
    {
        if ((rc = tf_ctrl_net_req_syn_handler[pmsg->msg_type](pmsg)))
        {
            printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        }
    }
    else
    {
        printf("%s %s %d req tf ctrl syn msg handler not support msg type %d!!! \n", __FILE__, __FUNCTION__, __LINE__, pmsg->msg_type);
    }

    return rc;
}

//接收同步请求消息处理任务
static void 
tf_ctrl_net_rx_syn_req_task(void *p_arg)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    UINT32          rc;
    UINT32          slot_id = *(UINT32*)p_arg;
    TF_NET_ZC_MQ   *p_mq = &g_tf_ctrl_rx_syn_req_mq[slot_id];
    
    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(p_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf("%s %s %d error %d slot_id %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc, slot_id);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;
     
        //处理消息
        if ((rc = tf_ctrl_net_msg_syn_req_process(p_usr_msg)))
        {
            printf("%s %s %d error %d slot_id %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc, slot_id);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);   
    }
    
    return;
}

//接收同步响应消息处理任务
static void 
tf_ctrl_net_rx_syn_ack_task(void *p_arg)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_usr_msg;
    UINT32          rc;
    UINT32          slot_id = *(UINT32*)p_arg;
    TF_NET_ZC_MQ   *p_mq   = &g_tf_ctrl_rx_syn_ack_mq[slot_id];

    while (1)
    {
        //队列消息 内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(p_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf("%s %s %d error %d slot %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc, slot_id);
            continue;
        }

        p_usr_msg = (TF_NET_MSG*)p_os_msg->data;

        //处理消息, conn 与slot对应
        if ((rc = tf_net_conn_msg_syn_ack_process(g_tf_ctrl_conn[slot_id], p_usr_msg)))
        {
            printf("%s %s %d error %d slot %d!!!\r\n", __FILE__, __FUNCTION__, __LINE__, rc, slot_id);
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);      
    }
    
    return;
}

//同步请求消息处理任务初始化
static UINT32 
tf_ctrl_net_rx_syn_req_task_init()
{
    pthread_t       tid;
    pthread_attr_t  attr;
    UINT32          rc;
    UINT32          slot_id;
    char            name[TF_TF_NET_NAME_LEN];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    //rx syn req queue & task
    for (slot_id = 0; slot_id < TF_NET_MANAGE_MAX_SLOT; slot_id++)
    {
        snprintf(name, sizeof(name), "server_rx_syn_req_mq_%d", slot_id);
        
        rc = tf_net_zc_mq_create(&g_tf_ctrl_rx_syn_req_mq[slot_id], name);
        if (rc)
        {
            printf("tf ctrl create rx syn req mq error %d slot %d\r\n", rc, slot_id);
            return TF_APP_ZK_RC_MSG_QUEUE_CREATE;
        }

        if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_rx_syn_req_task, (void *)&g_tf_ctrl_slot[slot_id]) < 0)
        {
            printf("tf ctrl create rx syn req task error %d slot %d\r\n", errno, slot_id);
            return TF_APP_ZK_RC_TASK_CREATE;
        }
    }
     
    return 0;
}

//同步响应消息处理任务初始化
static UINT32 
tf_ctrl_net_rx_syn_ack_task_init()
{
    pthread_t       tid;
    pthread_attr_t  attr;
    UINT32          rc;
    UINT32          slot_id;
    char            name[TF_TF_NET_NAME_LEN];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    //rx syn ack queue & task
    for (slot_id = 0; slot_id < TF_NET_MANAGE_MAX_SLOT; slot_id++)
    {        
        snprintf(name, sizeof(name), "server_rx_syn_ack_mq_%d", slot_id);
    
        rc = tf_net_zc_mq_create(&g_tf_ctrl_rx_syn_ack_mq[slot_id], name);
        if (rc)
        {
            printf("tf ctrl create rx syn ack mq error %d slot %d\r\n", rc, slot_id);
            return TF_APP_ZK_RC_MSG_QUEUE_CREATE;
        }

        if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_rx_syn_ack_task, (void*)&g_tf_ctrl_slot[slot_id]) < 0)
        {
            printf("tf ctrl create rx syn ack task error %d slot %d\r\n", errno, slot_id);
            return TF_APP_ZK_RC_TASK_CREATE;
        }
    }
 
    return 0;
}

#endif

#if DEF_FUNC("发送消息处理接口")

//发送板间消息任务
static void 
tf_ctrl_net_tx_task(void *p_arg)
{
    TF_NET_OS_MSG  *p_os_msg;
    TF_NET_MSG     *p_msg;
    UINT32          rc;
    UINT32          slot_id = *(UINT32*)p_arg;
    TF_NET_ZC_MQ   *p_mq   = &g_tf_ctrl_tx_mq[slot_id];

    while(1)
    {
        //内存由入队列端申请
        rc = tf_net_zc_mq_os_msg_get(p_mq, TF_NET_WAIT_FOREVER, &p_os_msg);
        if (rc)
        {
            printf("%s %s %d error %d\r\n", __FILE__, __FUNCTION__, __LINE__, rc);
            continue;
        }
        
        p_msg = (TF_NET_MSG*)p_os_msg->data;

        //msg header适配网络序，预留

        if (tf_net_send_alone_packet(SLOT2NET_FD(slot_id), p_msg))
        {
            printf("%s %s %d error!!!\n", __FILE__, __FUNCTION__, __LINE__);  
        }

        //释放队列消息
        tf_net_zc_mq_os_msg_free(p_os_msg);
    }

    return;
}

//发送板间消息队列和任务初始化
static UINT32 
tf_ctrl_net_tx_task_init()
{
    pthread_t       tid;
    pthread_attr_t  attr;
    UINT32          rc;
    UINT32          slot_id;
    char            name[TF_TF_NET_NAME_LEN];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);

    //tx queue & task
    for (slot_id = 0; slot_id < TF_NET_MANAGE_MAX_SLOT; slot_id++)
    {
        snprintf(name, sizeof(name), "server_tx_mq_%d", slot_id);
        
        rc = tf_net_zc_mq_create(&g_tf_ctrl_tx_mq[slot_id], name);
        if (rc)
        {
            printf("tf ctrl create tx mq error %d slot %d!!!\n", rc, slot_id);
            return TF_APP_ZK_RC_MSG_QUEUE_CREATE;
        }

        if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_tx_task, (void *)&g_tf_ctrl_slot[slot_id]) < 0)
        {
            printf("tf ctrl create tx task error %d slot %d!!!\n", errno, slot_id);
            return TF_APP_ZK_RC_TASK_CREATE;
        }
    }
    
    return 0;
}

#endif

#if DEF_FUNC("队列分发")

//接收板间消息异步分发处理
static void 
tf_ctrl_net_msg_asyn_mq_put(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = p_os_msg->data; 
    
    switch(p_usr_msg ->direction)
    {
        case NET_MSG_DRICTION_REQUEST:
            tf_net_zc_mq_os_msg_put(&g_tf_ctrl_rx_asyn_req_mq, p_os_msg);
            break;

        case NET_MSG_DRICTION_ACK:
            tf_net_zc_mq_os_msg_put(&g_tf_ctrl_rx_asyn_ack_mq, p_os_msg);
            break;

        default:  
            tf_net_zc_mq_os_msg_free(p_os_msg);      
            printf("%s %s %d tf ctrl not support msg direction!!! \r\n", __FILE__, __FUNCTION__, __LINE__);
            break;
    }
  
    return ;
}

//接收板间同步消息分发处理
static void 
tf_ctrl_net_msg_syn_mq_put(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = p_os_msg->data;
    UINT8       slot_id    = MSG2SLOT(p_usr_msg);

    switch(p_usr_msg ->direction)
    {
        case NET_MSG_DRICTION_REQUEST:
            tf_net_zc_mq_os_msg_put(&g_tf_ctrl_rx_syn_req_mq[slot_id], p_os_msg);
            break;

        case NET_MSG_DRICTION_ACK:
            tf_net_zc_mq_os_msg_put(&g_tf_ctrl_rx_syn_ack_mq[slot_id], p_os_msg);
            break;

        default:
            tf_net_zc_mq_os_msg_free(p_os_msg);     
            printf("%s %s %d tf ctrl not support msg direction!!! \r\n", __FILE__, __FUNCTION__, __LINE__);
            break;
    }
  
    return ;
}

//接收板间消息按类型入队列,入参合法性由上级保证
static void 
tf_ctrl_net_rx_mq_dispatch(TF_NET_OS_MSG *p_os_msg)
{
    TF_NET_MSG *p_usr_msg = (TF_NET_MSG*)p_os_msg->data;
    
    if (NET_MSG_ASYN == p_usr_msg->syn_flag)
    {
        tf_ctrl_net_msg_asyn_mq_put(p_os_msg);
    }
    else
    {
        tf_ctrl_net_msg_syn_mq_put(p_os_msg);
    }

    return ;
}

//接收板间消息入队列,入参合法性由上级保证
static void 
tf_ctrl_net_rx_mq_put(TF_NET_MSG *p_data)
{
    TF_NET_OS_MSG   *p_msg;
    
    //该申请bcmos msg由队列接收端释放
    p_msg = (TF_NET_OS_MSG*)tf_net_malloc(&g_tf_ctrl_tr_mq_mp, sizeof(TF_NET_OS_MSG));
    if (!p_msg)
    {
        tf_net_safe_free(p_data);
        printf("%s %s %d malloc fail\r\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    p_msg->data    = p_data;
    p_msg->size    = p_data->len;//队列中的消息体长度为板间消息有效长度

    //消息分发队列，整个过程由os接口托管
    tf_ctrl_net_rx_mq_dispatch(p_msg);

    return;
}

//发送板间消息入队列
static unsigned int 
tf_ctrl_net_tx_mq_put(TF_NET_MSG *p_data)
{
    TF_NET_OS_MSG  *p_msg;
    //申请msg内存由队列接收端释放
    p_msg = (TF_NET_OS_MSG*)tf_net_malloc(&g_tf_ctrl_tr_mq_mp, sizeof(TF_NET_OS_MSG));
    if (!p_msg)
    {
        tf_net_safe_free(p_data);
        printf("%s %s %d error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return TF_APP_ZK_RC_MEM_ALLOCATION;
    }

    //data 字段存储板间消息体
    p_msg->data     = p_data;
    p_msg->size     = p_data->len;
    
    tf_net_zc_mq_os_msg_put(&g_tf_ctrl_tx_mq[MSG2SLOT(p_data)], p_msg);
    
    return 0;
}

#endif

#if DEF_FUNC("网络公共接口")

//消息发送接口
static unsigned int 
tf_ctrl_net_msg_send(  TF_NET_MSG         *pmsg,
                            const void          *pvar, 
                            const unsigned int  len,
                            const int           state,
                            const int           direction)
{
    TF_NET_MSG      *ptmsg;
    unsigned int     rc;

    if (len)
    {
        ptmsg = tf_net_msg_pack(&g_tf_ctrl_net_msg_mp, pmsg->vif_info, pmsg->cmd_info, pvar, len, len);
    }
    else
    {
        ptmsg = tf_net_msg_pack(&g_tf_ctrl_net_msg_mp, pmsg->vif_info, pmsg->cmd_info, NULL, 0, 0);  
    }

    /*ptmsg 内存由发送端或者发送队列接口释放需要保证body_size 与 len一致*/
    if (!ptmsg)
    {
         printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
         return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    ptmsg->msg_type  = pmsg->msg_type;
    ptmsg->direction = direction;
    ptmsg->syn_flag  = pmsg->syn_flag;
    ptmsg->corr_tag  = pmsg->corr_tag;
    ptmsg->state     = state;
    
    if ((rc = tf_ctrl_net_tx_mq_put(ptmsg)))
    {
        printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return rc;
}

static UINT32 
tf_ctrl_net_init(int *p_fd)
{
    struct sockaddr_in  sin;
    int                 addrlen = sizeof(struct sockaddr);
    int                 fd;

    if (!p_fd)
    {
        return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    //建立网络fd，获取本板地址信息
    if (tf_net_init(&sin, &fd))
    {
        goto fail;
    }
    
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SERVER_PORT);
    
    if (bind(fd, (struct sockaddr *)&sin, addrlen) < 0)
    {
        close(fd);
        goto fail;
    } 

    *p_fd = fd;
    
    return 0;
    
fail:
    printf("%s %s %d error %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    return TF_APP_ZK_RC_NET_INIT;
}

//read 数据事件回调
static void 
tf_ctrl_net_read_cb(EV_P_ struct ev_io *watcher, int revents)
{  
    TF_NET_MSG   msg_header;
    TF_NET_MSG   *msg = NULL;
    unsigned int rc = EV_ERROR & revents;

    printf("control readcb events:%d\r\n", revents);
      
    if (rc)  
    {  
      printf("tf ctrl read event error revents %d\r\n", revents);
      
      if ((EV_ERROR^revents) == ERR_EV_LOCAL_FD_CLOSE)
        rc = TF_MDW_NET_RC_LOCAL_DISCONNECT;
        
      goto fail;
    }

    //1. 预读取消息头
    if ((rc = tf_net_rec_packet_fix_peek(watcher->fd, &msg_header, NET_MSG_HEAD_LEN)))
    {
        //断链或者其他
        goto fail;
    }
    
    //2. 头部大小端转换

    //3. 构造消息体 
    msg = tf_net_malloc(&g_tf_ctrl_net_msg_mp, msg_header.len);
    if (!msg)
    {
        printf("%s %s %d errorno %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        goto fail;
    }

    //4. 预读取消息体
    if ((rc = tf_net_rec_packet_fix_peek(watcher->fd, msg->body, (msg_header.len - NET_MSG_HEAD_LEN))))
    {
        //断链或者其他
        goto fail;
    }

    //5. 从缓存中取消息，消息体在处理端大小端转换
    if ((rc = tf_net_rec_packet_fix(watcher->fd, msg, msg_header.len)))
    {
        //断链错误
        goto fail;
    }

    //6. 头部大小端转换

    //7. 消息头校验
    if (tf_net_msg_verify(msg))
    {
        //失败，整个消息将丢弃
        printf("tf ctrl read cb msg verify error!!!:\n"
                "msg_type: %d\n"
                "cmd_id: %d\ntimeout: %d\norder: %d\n" 
                "vif_info: %lld\n"
                "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
                "state: %d\n"
                "len: %d\n"
                "compress: %d\n"
                "body_size: %d\n",
                msg->msg_type,
                msg->cmd_info.cmd_id,
                msg->cmd_info.timeout,
                msg->cmd_info.order,
                msg->vif_info,
                msg->syn_flag,
                msg->corr_tag,
                msg->direction, 
                msg->state, 
                msg->len,
                msg->compress,
                msg->body_size);
        
        goto fail;
    }

#if 0
    printf("ctrl rx cb msg info no peek:\n"
            "msg_type: %d\n"
            "cmd_id: %d\ntimeout: %d\norder: %d\n" 
            "vif_info:%llu\n"
            "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
            "state: %d\n"
            "len: %d\n"
            "compress: %d\n"
            "size: %d\n",
            msg->msg_type,
            msg->cmd_info.cmd_id,
            msg->cmd_info.timeout,
            msg->cmd_info.order,
            msg->vif_info,
            msg->syn_flag,
            msg->corr_tag,
            msg->direction, 
            msg->state, 
            msg->len,
            msg->compress,
            msg->body_size);
#endif   

    if (tf_net_msg_uncompress_process(&g_tf_ctrl_net_msg_mp, &msg))
    {
          printf("ctrl rx cb uncompress msg fail\r\n");
          goto fail;
    }

    //记录连接信息
    msg->fd = watcher->fd;
    //消息入接收队列缓存处理
    tf_ctrl_net_rx_mq_put(msg); 
    
    return ;
  
fail:

    tf_net_safe_free(msg);
  
    //业务面被动断链
    if(rc == TF_MDW_NET_RC_REMOTE_DISCONNECT)  
    {  
        printf("ctrl rx cb remote disconnect \r\n");
        tf_ctrl_net_inactive_disconnect_clear(loop, watcher);  
        //告警
    }
    else
    if (rc == TF_MDW_NET_RC_LOCAL_DISCONNECT)
    {
        //接入控制，业务面主动断链
        printf("ctrl rx cb local disconnect \r\n");
        tf_ctrl_net_active_disconnect_clear(loop, watcher);
    }
    else
    if (rc)
    {
        printf("tf ctrl read cb msg error!!!\r\n");
    }

    return ;
}

//accept事件 的回调块  
static void 
tf_ctrl_net_accept_cb(EV_P_ struct ev_io *watcher, int revents)  
{  
    struct sockaddr_in  client_addr;  
    socklen_t           client_len = sizeof(client_addr);  
    int                 client_fd; 
    struct ev_io        *w_client;
    uint32_t            map_idx;
   
    //libev的错误处理  
    if(EV_ERROR & revents)  
    {  
      printf("error event in accept\r\n");  
      return;  
    } 
    
    //与业务板端链接的ev io 
    client_fd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);  
    if (client_fd < 0)  
    {  
      printf("accept error\r\n");  
      return;  
    }  

    //访问控制预留， 地址黑名单
    //for debug
    printf("tfctrl someone connected. new fd %d\r\nip: %s\r\nport: %d\r\n", 
            client_fd,
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));

    if (tf_ctrl_net_map_free_id_get(&map_idx))
    {
        //没有空闲的slot映射id，丢弃该链接
        close(client_fd);
        printf("tf ctrl not have free connect\r\n");
        return;
    }

    w_client = (struct ev_io*)tf_net_malloc(&g_tf_ctrl_watcher_mp, sizeof(struct ev_io));
    if (!w_client)
    {
        close(client_fd);
        printf("%s %s %d error\r\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    //监听读事件
    ev_io_init(w_client, tf_ctrl_net_read_cb, client_fd, EV_READ);  
    ev_io_start(EV_A_ w_client);

    //初始化板间连接信息
    tf_ctrl_net_map_net_param_init(EV_A_ w_client, map_idx, &client_addr);

    return;
}

//定时事件
static void
tf_ctrl_net_timeout_cb (EV_P_ ev_timer *w, int revents)
{
    uint8_t        idx;
    
    for (idx = 0; idx < TF_NET_MANAGE_MAX_SLOT; idx++)
    {
        tf_ctrl_net_hb_timeout_process(idx);

        //close(g_tf_ctrl_net_map[tf_ctrl_net_map_id_get(idx)].net_param.fd);
        tf_ctrl_net_map_timeout_process(idx);
    }

    return ;
}

static void 
tf_ctrl_net_work_main(void)
{
    struct ev_loop  *loop = ev_default_loop(0);  
    int             fd;  
    ev_io           accept_watcher;  //监听事件
    ev_timer        timeout_watcher;//定时心跳检测

    //初始化控制板监听
    if (tf_ctrl_net_init(&fd))
    {
        return;
    }

    if (tf_net_setnonblock(fd))
    {
        printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
  
    if (listen(fd, 8) < 0)  
    {  
        printf("%s %s %d errorno %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return;  
    }

    //初始化conn 全局锁
    tf_net_mutex_create(&g_tf_ctrl_conn_lock);
    
    // 监听io事件
    ev_io_init(&accept_watcher, tf_ctrl_net_accept_cb, fd, EV_READ);  
    ev_io_start(loop, &accept_watcher);
    //定时事件
    ev_timer_init (&timeout_watcher, tf_ctrl_net_timeout_cb, 5, 5);
    ev_timer_start (loop, &timeout_watcher);

    ev_loop(loop, 0); 
}

//内存池初始化
UINT32 
tf_ctrl_net_mempool_init()
{
    //接收发送线程消息队列消息内存池
    TF_NET_BLK_POOL_PARM net_tr_mq_parm    = { "tf_ctrl_tr_mq_mp",
                                                sizeof(TF_NET_OS_MSG),
                                                NET_MSG_MAX_COUNT};

                                   
    TF_NET_BLK_POOL_PARM net_msg_parm      = {   "tf_ctrl_net_msg_mp",
                                                  NET_BUFFER_LEN,//(sizeof(TF_NET_MSG) + NET_BUFFER_LEN),
                                                  NET_MSG_MAX_COUNT};

    TF_NET_BLK_POOL_PARM net_watcher_parm  = { "tf_ctrl_net_watcher_mp",
                                                sizeof(union ev_any_watcher),
                                                NET_WATCHER_COUNT};

    TF_NET_BLK_POOL_PARM net_conn_parm     = { "tf_ctrl_net_conn_mp",
                                                sizeof(TF_NET_CONN),
                                                TF_NET_MANAGE_MAX_SLOT};

    TF_NET_BLK_POOL_PARM net_conn_msg_parm = {  "tf_ctrl_net_conn_msg_mp",
                                                 (sizeof(TF_NET_CONN_MSG)*TF_CTRL_NET_CONN_MSG_NUM),
                                                 TF_NET_MANAGE_MAX_SLOT};
    UINT32 rc = 0;
    
    rc |= tf_net_blk_pool_create(&g_tf_ctrl_tr_mq_mp,    &net_tr_mq_parm);
    rc |= tf_net_blk_pool_create(&g_tf_ctrl_net_msg_mp,  &net_msg_parm);
    rc |= tf_net_blk_pool_create(&g_tf_ctrl_watcher_mp,  &net_watcher_parm);
    rc |= tf_net_blk_pool_create(&g_tf_ctrl_conn_mp,     &net_conn_parm);
    rc |= tf_net_blk_pool_create(&g_tf_ctrl_conn_msg_mp, &net_conn_msg_parm);

    return rc;
}

UINT32 
tf_ctrl_net_task_init(void)
{
    UINT32 rc;
    //初始化网络接收和发送任务
    if((rc = tf_ctrl_net_rx_syn_req_task_init()))
    {
        printf("%s %s %d error !!!\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    };

    if((rc = tf_ctrl_net_rx_syn_ack_task_init()))
    {
        printf("%s %s %d error !!!\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    };

    if((rc = tf_ctrl_net_rx_asyn_req_task_init()))
    {
        printf("%s %s %d error !!!\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    };

    if((rc = tf_ctrl_net_rx_asyn_ack_task_init()))
    {
        printf("%s %s %d error !!!\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    };

    if((rc = tf_ctrl_net_tx_task_init()))
    {
        printf("%s %s %d error !!!\n", __FILE__, __FUNCTION__, __LINE__);
        return rc;
    };

    return 0;
}

//开辟线程启动异步事件驱动器进行网络监听
UINT32 
tf_ctrl_net_ev_init(void)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    
    if (pthread_create(&tid, &attr, (void *)tf_ctrl_net_work_main, NULL) < 0)
    {
        printf("%s %s %d errorno %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        return TF_APP_ZK_RC_TASK_CREATE;
    }
    
    return 0;
}

#endif

#if DEF_FUNC("接收异步请求消息处理接口")

static UINT32 
tf_ctrl_net_asyn_req_work_process(TF_NET_MSG *pmsg)
{     
    uint32_t        rc;
    TF_NET_PHY_INFO phy_info;
    uint32_t        slot_id;
    
    //获取消息基础参数
    if((rc = tf_net_vif2phy(pmsg->vif_info, &phy_info)))
    {
        printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        return rc;
    }

    slot_id = TF_NET_PHY_INFO_TO_SLOT_ID(phy_info);

    //同步板间链路状态
    if ((rc = tf_ctrl_net_work_synch_process(pmsg, slot_id)))
    {
        return rc;
    }
     
    //响应业务板暂时不带负荷 
    tf_ctrl_net_msg_send(pmsg, NULL, 0, 0, NET_MSG_DRICTION_ACK);
  
    //通知其他进程tf启动完毕
    //使能控制板cli

    return 0;  
}

static UINT32 
tf_ctrl_net_asyn_req_hb_process(TF_NET_MSG *pmsg)
{  
    unsigned int        rc;
    //tf_rc_info         rc_info = {.obj_id = OBJ_APP_STAT};
    TF_NET_PHY_INFO    phy_info;
    uint8_t             slot_id;
    //const NIF_LOAD_INFO *p_load_info;

    if((rc = tf_net_vif2phy(pmsg->vif_info, &phy_info)))
    {
        printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        return rc;
    }

    slot_id = TF_NET_PHY_INFO_TO_SLOT_ID(phy_info);

    //记录状态清超时
    tf_ctrl_net_hb_cnt_set(slot_id, 0);
    printf("zk rec heartbeat\r\n");
    
#if 0    
    //获取单板负荷信息
    if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_LOAD_INFO))
    {
       rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
       tf_info2rc_convert(&rc, &rc_info);
       goto end;
    }
    else
    {
        p_load_info = (const NIF_LOAD_INFO *)(void*)pmsg->body;
        //printf("tf ctrl rec hb slot %d mem load %d%c cpu load %d%c\r\n", 
                //slot_id, 
                //p_load_info->mem_load, '%',
                //p_load_info->cpu_load, '%');
        if (p_load_info->mem_load > 95 || p_load_info->cpu_load > 95)
        {
            printf("tf ctrl rec hb slot %d mem load %d%c cpu load %d%c\r\n", 
                    slot_id, 
                    p_load_info->mem_load, '%',
                    p_load_info->cpu_load, '%');
        }
    }
#endif

//end:

    return rc;
}

static UINT32 
tf_ctrl_net_asyn_req_auto_process(TF_NET_MSG *pmsg)
{
    pmsg = pmsg;
    //1.当前记录状态
    
    return 0;
}

static UINT32 
tf_ctrl_net_alarm_asyn_req_alarm_process(TF_NET_MSG *pmsg)
{
    unsigned int       rc;
    //tf_rc_info         rc_info = {.obj_id = OBJ_APP_ALARM};
    TF_NET_PHY_INFO    phy_info;
    //TF_NET_ALARM_INFO  alarm_info;
    //uint8_t             dev_id;
    
#if 0
    printf("ctrl rx alarm msg info:\r\n"
              "msg_type: %d\n"
              "cmd_id: %d\ntimeout: %d\n" 
              "vif_info: %lld\n"
              "syn_flag: %d\ncor_tag: %d\ndir: %d\n"
              "state: %d\n"
              "len: %d\n"
              "body_size: %d\n",
              msg->msg_type,
              msg->cmd_info.cmd_id,
              msg->cmd_info.timeout,
              msg->vif_info,
              msg->syn_flag,
              msg->corr_tag,
              msg->direction, 
              msg->state, 
              msg->len,
              msg->body_size);
 #endif
 
    //获取消息基础参数
    if((rc = tf_net_vif2phy(pmsg->vif_info, &phy_info)))
    {
        printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
        return rc;
    }
    
    //dev_id = TF_NET_PHY_INFO_TO_DEV_ID(phy_info);
/*    alarm_info.slot_id      = TF_NET_PHY_INFO_TO_SLOT_ID(phy_info);
    alarm_info.port_id       = TF_NET_PHY_INFO_TO_TF_ID(phy_info);
    alarm_info.node_id      = TF_NET_PHY_INFO_TO_ONU_ID(phy_info);
    alarm_info.alarm_id     = pmsg->cmd_info.cmd_id;
    alarm_info.alarm_state  = NET_ALARM_STATE_REPORT;*/

    switch(pmsg->cmd_info.cmd_id)
    {
#if 0    
        case NET_MSG_CMD_ALARM_ONU_OFFLINE:
            {
                const NIF_ONU_BASE_INFO_ALARM *p_onu_base_info_alarm;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_ONU_BASE_INFO_ALARM))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    p_onu_base_info_alarm = (const NIF_ONU_BASE_INFO_ALARM *)(void*)pmsg->body;
                }
                
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID | NET_ALARM_PARA_TYPE_ONU_SN;//NET_ALARM_PARA_TYPE_ONU_PWD
                memcpy(alarm_info.phy_addr, p_onu_base_info_alarm->sn, sizeof(alarm_info.phy_addr));
                memcpy(alarm_info.pwd, p_onu_base_info_alarm->pw, sizeof(alarm_info.pwd));

                if (pmsg->cmd_info.cmd_id == NET_MSG_CMD_ALARM_ONU_ONLINE)
                {
                    //printf("tf ctrl led online alarm!!! pre slot %d, tf %d num %d\n", alarm_info.slot_id, alarm_info.port_id, tf_ctrl_net_port_onu_online_get(alarm_info.slot_id, alarm_info.port_id));
                    //第一个上线亮灯
                    if (!tf_ctrl_net_port_onu_online_get(alarm_info.slot_id, alarm_info.port_id))
                    {

                    }
                    
                    tf_ctrl_net_port_onu_online_add(alarm_info.slot_id, alarm_info.port_id);
                }
                else
                if (pmsg->cmd_info.cmd_id == NET_MSG_CMD_ALARM_ONU_OFFLINE)
                {
                    //printf("tf ctrl led offline alarm pre slot %d, tf %d num %d\n", alarm_info.slot_id, alarm_info.port_id, tf_ctrl_net_port_onu_online_get(alarm_info.slot_id, alarm_info.port_id));
                    //全部下线需要灭灯
                    tf_ctrl_net_port_onu_online_dec(alarm_info.slot_id, alarm_info.port_id);
                    if (!tf_ctrl_net_port_onu_online_get(alarm_info.slot_id, alarm_info.port_id))
                    {

                    }
                }
            }
            break;
            
        case NET_MSG_CMD_ALARM_ONU_RANGE_FAIL:
            {
                const NIF_ONU_RANGE_FAIL_ALARM *p_onu_range_alarm;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_ONU_RANGE_FAIL_ALARM))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    p_onu_range_alarm = (const NIF_ONU_RANGE_FAIL_ALARM *)(void*)pmsg->body;
                }
                
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID | NET_ALARM_PARA_TYPE_ONU_SN;//NET_ALARM_PARA_TYPE_ONU_PWD
                memcpy(alarm_info.phy_addr, p_onu_range_alarm->onu_base_info.sn, sizeof(alarm_info.phy_addr));
                memcpy(alarm_info.pwd, p_onu_range_alarm->onu_base_info.pw, sizeof(alarm_info.pwd));
                //printf("tf ctrl onu range fail reasion: %d\n", p_onu_range_alarm->range_fail_reasion);
            }
            break;
            
        case NET_MSG_CMD_ALARM_ONU_ACTIVE_FAIL:
            {
                const NIF_ONU_ACTIVATE_FAIL_ALARM *p_onu_activate_fail;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_ONU_ACTIVATE_FAIL_ALARM))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    p_onu_activate_fail = (const NIF_ONU_ACTIVATE_FAIL_ALARM *)(void*)pmsg->body;
                }
                
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID | NET_ALARM_PARA_TYPE_ONU_SN;//NET_ALARM_PARA_TYPE_ONU_PWD
                memcpy(alarm_info.phy_addr, p_onu_activate_fail->onu_base_info.sn, sizeof(alarm_info.phy_addr));
                memcpy(alarm_info.pwd, p_onu_activate_fail->onu_base_info.pw, sizeof(alarm_info.pwd));
                printf("tf ctrl onu active fail reasion: %d\n", p_onu_activate_fail->fail_reasion);
            }
            break;
            
        case NET_MSG_CMD_ALARM_ONU_PWD_AUTH_FAIL:
            {
                const NIF_ONU_PASSWORD_AUTH_FAIL_ALARM *p_onu_password_auth_fail;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_ONU_PASSWORD_AUTH_FAIL_ALARM))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    p_onu_password_auth_fail = (const NIF_ONU_PASSWORD_AUTH_FAIL_ALARM *)(void*)pmsg->body;
                }
                
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID | NET_ALARM_PARA_TYPE_ONU_SN;//NET_ALARM_PARA_TYPE_ONU_PWD
                memcpy(alarm_info.phy_addr, p_onu_password_auth_fail->onu_base_info.sn, sizeof(alarm_info.phy_addr));
                memcpy(alarm_info.pwd, p_onu_password_auth_fail->onu_base_info.pw, sizeof(alarm_info.pwd));
                //printf("tf ctrl onu active fail reasion: %d\n", p_onu_password_auth_fail->range_fail_reasion);
            }
            break;
           
        case NET_MSG_CMD_ALARM_ONU_DF:
            {
                const NIF_TF_ONU_ALARM_STATE *tf_onu_alarm_state;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_TF_ONU_ALARM_STATE))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    tf_onu_alarm_state = (const NIF_TF_ONU_ALARM_STATE *)(void*)pmsg->body;
                }
                
                alarm_info.alarm_state  = tf_onu_alarm_state->onu_alarm_state;
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID | NET_ALARM_PARA_TYPE_ONU_SN;//NET_ALARM_PARA_TYPE_ONU_PWD
                memcpy(alarm_info.phy_addr, tf_onu_alarm_state->onu_base_info.sn, sizeof(alarm_info.phy_addr));
                memcpy(alarm_info.pwd, tf_onu_alarm_state->onu_base_info.pw, sizeof(alarm_info.pwd));
            }
            break; 
            
        case NET_MSG_CMD_ALARM_TF_STATE_CHANGE:
            {
                const NIF_TF_NI_STATE_CHANGE *tf_ni_state_change;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_TF_NI_STATE_CHANGE))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);\
                    goto end;
                }
                else
                {
                    tf_ni_state_change = (const NIF_TF_NI_STATE_CHANGE*)(void*)pmsg->body;
                }

                //目前只关注tf ni deactive 相关的状态变化
                if (tf_ni_state_change->result == 0)
                {
                    if (tf_ni_state_change->previous_state != 0 && tf_ni_state_change->new_state == 0)
                    {
                        alarm_info.alarm_state = NET_ALARM_STATE_REPORT;
                    }
                    else
                    if (tf_ni_state_change->previous_state == 0 && tf_ni_state_change->new_state == 2)
                    {
                        alarm_info.alarm_state = NET_ALARM_STATE_CLEAR;
                    }
                    else
                    {
                        //other sate change
                        goto end;
                    }
                }
                else
                {
                    //state change not suc
                    goto end;
                }
                
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID;
            }
            break;
            
        case NET_MSG_CMD_ALARM_LOS:
            {
                const uint32_t *tf_ni_los_state;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(uint32_t))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);
                    goto end;
                }
                else
                {
                    tf_ni_los_state = (const uint32_t*)(void*)pmsg->body;
                }
                
                alarm_info.alarm_state  = *tf_ni_los_state;
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT | NET_ALARM_PARA_TYPE_TFID;
            }
            break;
            
        case NET_MSG_CMD_ALARM_HOST_CONNECT_FAIL:
            {
                const NIF_HOST_CONNECTION_FAIL_REASON *p_fail_reasion;
                if (pmsg->len - NET_MSG_HEAD_LEN != sizeof(NIF_HOST_CONNECTION_FAIL_REASON))
                {
                    rc_info.err_id = TF_APP_TFALARM_RC_PARAM_ONT_OF_RANGE;
                    tf_info2rc_convert(&rc, &rc_info);
                    goto end;
                }
                else
                {
                    p_fail_reasion = (const NIF_HOST_CONNECTION_FAIL_REASON*)(void*)pmsg->body;
                }
                //68620连接重新连接成功 清除
                printf("connect device fail reasion: %d!!!\n", *p_fail_reasion);
                alarm_info.alarm_state  = NET_ALARM_STATE_REPORT;
                alarm_info.para_bit_map = NET_ALARM_PARA_TYPE_SLOT;
            }
            break;
#endif            
        default:
            {
                goto end;
            }
        
    }
    
    //rc = tf_ctrl_alarm_process(&alarm_info);
    
end:
    
    return rc;
}

#endif

#if DEF_FUNC("接收异步响应消息处理接口")

static UINT32 
tf_ctrl_net_asyn_ack_onu_set_process(TF_NET_MSG *pmsg)
{
    pmsg = pmsg;

    //1.当前记录状态

    return 0;
}

static UINT32 
tf_ctrl_net_asyn_ack_onu_get_process(TF_NET_MSG *pmsg)
{
    pmsg = pmsg;
    //1.当前记录状态

    return 0;
}

static UINT32 
tf_ctrl_net_asyn_ack_olt_set_process(TF_NET_MSG *pmsg)
{
    pmsg = pmsg;

    //1.当前记录状态
    
    return 0;
}

static UINT32 
tf_ctrl_net_asyn_ack_olt_get_process(TF_NET_MSG *pmsg)
{
    //for debug
    pmsg = pmsg;

    return 0;
}

static UINT32 
tf_ctrl_net_asyn_ack_nw_set_process(TF_NET_MSG *pmsg)
{
    //for debug
    pmsg = pmsg;

    //1.当前记录状态

    return 0;
}

static UINT32 
tf_ctrl_net_asyn_ack_nw_get_process(TF_NET_MSG *pmsg)
{
    //for debug
    pmsg = pmsg;

    //1.当前记录状态

    return 0;
}

#endif

#if DEF_FUNC("接收同步请求消息处理接口")

static UINT32 
tf_ctrl_net_syn_req_drv_pull_process(TF_NET_MSG *p_msg)
{
    /*参数名称和标志msg,rc,rc_info,p_buf,len,ack涉及宏，慎重修改*/
    unsigned int        rc;
    tf_rc_info          rc_info = {.obj_id = OBJ_APP_ZK, .sub_obj_id.olt_id = MSG2SLOT(p_msg)};
    void                *p_buf = NULL;
    unsigned int        len = 0;
    
    TF_NET_PHY_INFO    phy_info;
    //uint8_t             dev_id;
    //uint8_t             port_id;
    //uint16_t            onu_id;
    
    //获取消息基础参数
    if ((rc = tf_net_vif2phy(p_msg->vif_info, &phy_info)))
    {
      printf("%s %s %d error %d!!! \n", __FILE__, __FUNCTION__, __LINE__, rc);
      goto ack;
    }
    
    //dev_id = TF_NET_PHY_INFO_TO_DEV_ID(phy_info);
    //port_id = TF_NET_PHY_INFO_TO_TF_ID(phy_info);
    //onu_id = TF_NET_PHY_INFO_TO_ONU_ID(phy_info);

    switch(p_msg->cmd_info.cmd_id)
    {
        case NET_MSG_CMD_PROFILE_GET:
            {   
                char *buffer = tf_net_malloc(&g_tf_ctrl_net_msg_mp, MSG_BODY_LEN_TEST_PC);
                memset(buffer, 0x55, MSG_BODY_LEN_TEST_PC);
                p_buf = buffer;
                len = MSG_BODY_LEN_TEST_PC;
                sleep(1);//拥塞测试
            }
            break;

        default:
            TF_TF_ERRNO_INFO2RC(rc_info, rc, TF_APP_ZK_RC_PARAM_OUT_OF_RANGE);
            break;      
    }

    //no error return 0
    if (!(rc%TF_RETURNCODE_BASE))rc = 0;

ack:
    if (rc)
    {
        printf("%s %s %d msg %d error %d\n", __FILE__, __FUNCTION__, __LINE__, p_msg->cmd_info.cmd_id, rc%TF_RETURNCODE_BASE);
    }
    
    tf_ctrl_net_msg_send(p_msg, p_buf, len, rc, NET_MSG_DRICTION_ACK);
    tf_net_safe_free(p_buf);
    
    return 0;
}

#endif


#if DEF_FUNC("发送异步请求消息接口")

UINT32 
tf_ctrl_net_asyn_operation(IN const TF_NET_MSG_TYPE   msg_type,
                                IN const UINT64             vif_info, 
                                IN const TF_NET_MSG_CMD    cmd_info, 
                                IN const void               *pvar, 
                                IN const UINT32             len)
{
    TF_NET_MSG *msg = tf_net_msg_pack(&g_tf_ctrl_net_msg_mp, vif_info, cmd_info, pvar, len, len);

    if (!msg)
    {
        printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
        return TF_APP_ZK_RC_MEM_ALLOCATION;
    }
    
    msg->msg_type  = msg_type;
    msg->direction = NET_MSG_DRICTION_REQUEST;
    msg->syn_flag  = NET_MSG_ASYN;
    
    if (tf_ctrl_net_tx_mq_put(msg))
    {
        printf("%s %s %d error msg_type: %d\n", __FILE__, __FUNCTION__, __LINE__, msg->msg_type);
    }

    return 0;  
}

#endif

#if DEF_FUNC("发送同步消息接口")

static UINT32 
tf_ctrl_net_conn_msg_list_alloc(TF_NET_CONN *conn)
{
    int i;
    TF_NET_CONN_MSG *ptmsg;
    
    conn->msg_array = tf_net_calloc(&g_tf_ctrl_conn_msg_mp, conn->msg_num, 
                        sizeof(TF_NET_CONN_MSG));
   
    if (!conn->msg_array)
        return TF_APP_ZK_RC_MEM_ALLOCATION;

    ptmsg = conn->msg_array;
    for (i = 0; i < conn->msg_num; i++, ptmsg++)
    {
        UINT32 rc;

        TAILQ_INSERT_TAIL(&conn->free_req_list, ptmsg, l);
        
        rc = tf_net_sem_create(&ptmsg->sem, 0);
        if (rc)
            return rc;
    }

    return 0;
}

static UINT32 
tf_ctrl_net_conn_init(INT32 slot_id, TF_NET_CONN **pconn)
{
    TF_NET_CONN *conn;

    tf_net_mutex_lock(&g_tf_ctrl_conn_lock);
    conn = tf_net_calloc(&g_tf_ctrl_conn_mp, 1, sizeof(*conn));
    
    if (!conn)
    {
        tf_net_mutex_unlock(&g_tf_ctrl_conn_lock);
        return TF_APP_ZK_RC_MEM_ALLOCATION;
    }

    snprintf(conn->name, sizeof(conn->name), "tf_slot_%u", slot_id);
    TAILQ_INIT(&conn->free_req_list);
    TAILQ_INIT(&conn->msg_list);
    tf_net_mutex_create(&conn->mutex);
    conn->msg_num = TF_CTRL_NET_CONN_MSG_NUM;

    tf_ctrl_net_conn_msg_list_alloc(conn);

    conn->connected = 1;
    conn->slot_id = slot_id;
    *pconn = conn;
    
    tf_net_mutex_unlock(&g_tf_ctrl_conn_lock);

    return 0;
}

static UINT32 
tf_ctrl_net_conn_get_any(INT32 slot_id, TF_NET_CONN **pconn)
{
    UINT32 err;

    if (!pconn)
    {
        return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    if (slot_id >= TF_NET_MANAGE_MAX_SLOT)
    {
        return TF_APP_ZK_RC_PARAM_OUT_OF_RANGE;
    }
    
    *pconn = g_tf_ctrl_conn[slot_id];
    
    if (*pconn)
    {
        return 0;
    }
    
    err = tf_ctrl_net_conn_init(slot_id, &g_tf_ctrl_conn[slot_id]);
    *pconn = g_tf_ctrl_conn[slot_id];
    
    return err;
}

UINT32 
tf_ctrl_net_conn_get(INT32 slot_id, TF_NET_CONN **pconn)
{
    return tf_ctrl_net_conn_get_any(slot_id, pconn);
}

/* Send message and wait for restfse */
static UINT32 
tf_ctrl_net_msg_syn_send(TF_NET_MSG *pmsg)
{
    if (!pmsg)
    {
        return TF_APP_ZK_RC_PARAM_NULL;
    }
    
    TF_NET_CONN_MSG *pcmsg;
    TF_NET_CONN     *pconn;
    unsigned int     err;

    //根据slot获取conn
    err = tf_ctrl_net_conn_get(MSG2SLOT(pmsg), &pconn);
    if (err)
    {
        printf("tf_ctrl_net_conn_get error!!!\n");
        return err;
    }

    /* request under connection lock */
    tf_net_mutex_lock(&pconn->mutex);

    pcmsg = tf_net_conn_msg_get_free(&pconn->free_req_list); 
    if (!pcmsg)
    {
        tf_net_mutex_unlock(&pconn->mutex);
        printf("tf_ctrl_net_conn_msg_get_free error!!!\n");
        return TF_APP_ZK_RC_PARAM_GET;
    }

    //时间戳作为接收校验魔术字
    pmsg->corr_tag = tf_net_timestamp();

    //拷贝用于发送消息，本体用于同步消息接收
    TF_NET_MSG *p_autofree_msg = tf_net_malloc(&g_tf_ctrl_net_msg_mp, pmsg->len);
    if (!p_autofree_msg)
    {
        printf("%s %s %d tf_net_malloc error!!!\r\n", __FILE__, __FUNCTION__, __LINE__);
        tf_net_conn_msg_free(pcmsg, &pconn->free_req_list);
        return TF_APP_ZK_RC_MEM_ALLOCATION;
    }
    
    memcpy(p_autofree_msg, pmsg, pmsg->len);
    
    //pmsg挂conn 消息用于接收
    pcmsg->data = pmsg;

    //发送任务
    if ((err = tf_ctrl_net_tx_mq_put(p_autofree_msg)))
    {
        printf("%s %s %d error!!!\n", __FILE__, __FUNCTION__, __LINE__);
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
        
        return TF_APP_ZK_RC_MSG_SYN_WAITE;
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
UINT32 
tf_ctrl_net_syn_operation( IN const  TF_NET_MSG_TYPE msg_type,
                                IN const  UINT64           vif_info, 
                                IN const  TF_NET_MSG_CMD  cmd_info, 
                                IN const  void             *pvar, 
                                IN const  UINT32           pvar_len,
                                OUT void                   *param,
                                IN  const UINT32           param_len,
                                OUT UINT32                 *state)
{
    UINT32      syn_ack_len;
    TF_NET_MSG *p_msg;
    UINT32      rc;
    tf_rc_info rc_info = {.obj_id = OBJ_APP_ZK, .sub_obj_id.olt_id = (vif_info & 0xff)};

    //首先判断链接是否存在
    if (!tf_ctrl_net_state_working(vif_info & 0xff))
    {
        printf("tf ctrl remote slot %llu state is not working\r\n", vif_info & 0xff);
        return TF_OBJ_APP_TF_ZK_RC_OFFSET + TF_APP_ZK_RC_NET_NOT_CONNECT;
    }
    
    if (!state)
    {
        TF_TF_ERRNO_INFO2RC(rc_info, rc, TF_APP_ZK_RC_PARAM_NULL);
        return rc;
    }
    
    if ((param && 0 == param_len) || (NULL == param && param_len))
    {
        printf("%s %s %d param error\n", __FILE__, __FUNCTION__, __LINE__);
        TF_TF_ERRNO_INFO2RC(rc_info, rc, TF_APP_ZK_RC_PARAM_OUT_OF_RANGE);
        return rc;
    }
    
    if ((NULL == param && 0 == param_len) || (param_len < pvar_len))
    {
        //需要传参但不需要返回参数|| 传参结构大于返回参数
        p_msg = tf_net_msg_pack(&g_tf_ctrl_net_msg_mp, vif_info, cmd_info, pvar, pvar_len, pvar_len);
    }
    else
    {
        //传参结构小于返回参数
        p_msg = tf_net_msg_pack(&g_tf_ctrl_net_msg_mp, vif_info, cmd_info, pvar, pvar_len, param_len);
    }
   
    if (!p_msg)
    {
        printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
        TF_TF_ERRNO_INFO2RC(rc_info, rc, TF_APP_ZK_RC_MEM_ALLOCATION);
        return rc;
    }
    
    p_msg->msg_type  = msg_type;
    p_msg->direction = NET_MSG_DRICTION_REQUEST;
    p_msg->syn_flag  = NET_MSG_SYN;
    
    if ((rc = tf_ctrl_net_msg_syn_send(p_msg)))
    {
        printf("%s %s %d error\n", __FILE__, __FUNCTION__, __LINE__);
        
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
            printf("tf ctrl struct msg, body size too small!!!\n");

            memcpy(param, p_msg->body, (syn_ack_len > param_len ? param_len : syn_ack_len));
        }
    }
  
    tf_net_safe_free(p_msg);

    return 0;  
}

#endif

