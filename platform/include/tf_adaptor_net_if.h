 /**************************************************************
 * 文件名称: 
 * 文件描述:
 * 版           本: 
 * 修改历史:
 *     <修改者>     <时间>      <版本 >     <描述>

 **************************************************************/
#ifndef __TF_ADAPTOR_NET_IF_H__
#define __TF_ADAPTOR_NET_IF_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include "tf_adaptor_common_if.h"
#include "tf_adaptor_errno.h"


//for debug Performance Testing
#define MSG_BODY_LEN_TEST_PC  (1024*3)
#define MSG_COUNT_PC_TEST     10000

typedef struct
{
    char buffer[MSG_BODY_LEN_TEST_PC];
}MSG_PC_TEST;
//for debug Performance Testing


#define IO_MAXSIZE              100
#define TF_NET_SEND_COUNT      3

#define SERVER_PORT             2000
#define CLIENT_PORT             4000

#define ERR_EV_LOCAL_FD_CLOSE   0x3
#define MSG2SLOT(p_msg)         (p_msg->vif_info & 0xff)


#define SLOT_ID_HOLD_BITS       8
#define TF_NI_HOLD_BITS         8
#define ONU_ID_HOLD_BITS        8
#define ALLOC_ID_HOLD_BITS      16
#define GEMPORT_ID_HOLD_BITS    16
#define VLAN_ID_HOLD_BITS       16
#define FLOW_ID_HOLD_BITS       16
#define GPIO_ID_HOLD_BITS       8
    
#define KEY_TYPE_MOVE_BITS      56
    
typedef enum TF_NET_PHY_TYPE
{
    TF_NI = 1,
    TF_ONU,
    TF_ALLOC,
    TF_GEMPORT,
    TF_VLAN,
    TF_FLOW,
    TF_GPIO,
    TF_BOARD,

    NET_PHY_TYPE_NUM_OF
}TF_NET_PHY_TYPE;

typedef struct TF_NET_PHY_INFO{
    TF_NET_PHY_TYPE key_type;
    /* 主键key 类型*/
    struct {
        unsigned int slot_id;        /*use 8bit*/
        unsigned int tf_ni;          /*use 8bit*/
        unsigned int onu_id;         /*use 8bit*/
        unsigned int alloc_id;       /*use 16bit*/
        unsigned int gemport_id;     /*use 16bit*/
        unsigned int vlan_id;        /*use 16bit*/
        unsigned int flow_id;        /*use 16bit*/
        unsigned int gpio_id;        /*use 8bit*/
    } phy_oscs;
} TF_NET_PHY_INFO;

#define TF_NET_PHY_INFO_TO_SLOT_ID(phy_info)  ((phy_info).phy_oscs.slot_id)
#define TF_NET_PHY_INFO_TO_DEV_ID(phy_info)   ((phy_info).phy_oscs.tf_ni / 16)
#define TF_NET_PHY_INFO_TO_TF_ID(phy_info)    ((phy_info).phy_oscs.tf_ni % 16)
#define TF_NET_PHY_INFO_TO_ONU_ID(phy_info)   ((phy_info).phy_oscs.onu_id)

//msg type
typedef enum
{
    NET_MSG_TYPE_WORK = 1,      /*开工non-zero postive*/
    NET_MSG_TYPE_HB,            /*心跳*/
    NET_MSG_TYPE_ONU_SET,       /*onu配置*/
    NET_MSG_TYPE_ONU_GET,       /*onu状态获取*/
    NET_MSG_TYPE_OLT_SET,       /*olt配置*/
    NET_MSG_TYPE_OLT_GET, 
    NET_MSG_TYPE_NW_SET,
    NET_MSG_TYPE_NW_GET,
    NET_MSG_TYPE_AUTO_REPORT,   /*自发消息上报*/
    NET_MSG_TYPE_ALARM_REPORT,  /*告警信息上报*/
    NET_MSG_TYPE_DRV_PULL,      /*drv 层获取ctrl层内容*/ 
    
    NET_MSG_TYPE_NUM_OF
}TF_NET_MSG_TYPE;

typedef enum
{
    NET_MSG_ORDER_IN,
    NET_MSG_ORDER_OUT_OF,
    
    NET_MSG_ORDER_NUM_OF,
}TF_NET_MSG_ORDER;

typedef struct
{
    int    cmd_id;       /*命令类型*/
    int    timeout;      /*消息回应等待超时该参数只在ctrl同步消息时有效*/
    int    order;        /*消息是否需要按照时序执行*/
}TF_NET_MSG_CMD;

//cmd id
typedef enum
{
    NET_MSG_CMD_WORK = 1,
    NET_MSG_CMD_REWORK,  /*客户端重连*/

    NET_MSG_CMD_WORK_NUM_OF  
}TF_NET_MSG_CMD_WORK;

typedef enum
{
    NET_MSG_CMD_HB = 1,
    NET_MSG_CMD_HB_LOAD,

    NET_MSG_CMD_HB_NUM_OF 
}TF_NET_MSG_CMD_HB;

typedef enum
{
    NET_MSG_CMD_PROFILE_GET = 1,

    NET_MSG_CMD_PROFILE_NUM_OF 
}TF_NET_MSG_CMD_DRV_MIX_GET;

//syn flag
typedef enum
{
    NET_MSG_ASYN = 1,
    NET_MSG_SYN,
    
    NET_MSG_SYN_NUM_OF
}TF_NET_MSG_SYN;

//msg dir
typedef enum
{
    NET_MSG_DRICTION_REQUEST = 1,
    NET_MSG_DRICTION_ACK,

    NET_MSG_DRICTION_NUM_OF
}TF_NET_DRICTION;

//net msg
typedef struct
{
    TF_NET_MSG_TYPE     msg_type;  /*消息类型*/
    TF_NET_MSG_CMD      cmd_info;  /*cmd*/
    unsigned long long  vif_info;
    TF_NET_MSG_SYN      syn_flag;  /*是否同步消息*/
    unsigned int        corr_tag;  /*Correlation tag */
    TF_NET_DRICTION     direction; /*消息方向*/
    int                 state;     /*业务面返回执行的状态*/
    unsigned int        len;       /* msg有效长度*/
    unsigned int        compress;  /*数据是否压缩, 该参数用户无需关注*/
    unsigned int        uncmp_len; /*数据压缩前的长度，该参数用户无需关注*/
    unsigned int        fd;        /*消息传递描述字，该参数用户无需关注*/
    unsigned int        body_size; /*消息结构(body)长度该参数用户无需关注*/
    char                body[0];   /*消息体*/
}TF_NET_MSG;

#define NET_MSG_HEAD_LEN (sizeof(TF_NET_MSG))

typedef unsigned int (*net_msg_handler)(TF_NET_MSG *pmsg);


//通信接口请求消息处理返回码通用宏
//有传参
#define TF_TF_NET_PROCESS_REQ_PARAM_STRUCT(type,p_name,msg,rc_info,ret,err_no,end)  do{\
      if (msg->len - NET_MSG_HEAD_LEN != sizeof(type))\
      {\
            TF_TF_ERRNO_INFO2RC(rc_info, ret, err_no);\
            goto end;\
      }\
      else\
      {\
             p_name = (type*)(void*)(msg->body);\
      }\
    }while(0)

//没有传参
#define TF_TF_NET_PROCESS_REQ_NO_PARAM_STRUCT(msg,rc_info,ret,err_no,end) do{\
        if ((msg)->len - NET_MSG_HEAD_LEN != 0)\
        {\
            TF_TF_ERRNO_INFO2RC(rc_info, ret, err_no);\
            goto end;\
        }\
    }while(0)

//有回传参数
#define TF_TF_NET_PROCESS_ACK_PARAM_STRUCT(type,p_name,pool_name,p_buf,buf_len,rc_info,ret,err_no,end)   do{\
        p_name = (type*)tf_net_malloc(&pool_name, sizeof(type)); \
        if (!p_name)    \
        {\
            TF_TF_ERRNO_INFO2RC(rc_info, ret, err_no);\
            goto end;\
        }\
        p_buf   = p_name;\
        buf_len = sizeof(type);\
    }while(0)


//板间消息支持多消息组合
typedef struct
{
    int          count;
    struct iovec io[IO_MAXSIZE];
}TF_NET_MULTI_MSG;

//板间连接信息
typedef struct
{
    struct ev_loop    *loop;
    struct ev_io      *watcher;
    struct sockaddr_in remote_addr;//远端地址
}TF_NET_PARA;

typedef TAILQ_HEAD(tf_net_conn_list, TF_NET_CONN_MSG) tf_net_conn_list;

typedef struct TF_NET_CONN_MSG
{
    TF_NET_SEM                   sem;                 /* "wait for restfse" semaphore */
    TAILQ_ENTRY(TF_NET_CONN_MSG) l;                   /*Transport message list entry */
    void                         *data;               /*pointer to TF_NET_MSG*/
    
}TF_NET_CONN_MSG;

typedef struct 
{
    char                  name[30];            /*name */
    uint8_t               slot_id;             /*slot index */
    TF_NET_CONN_MSG       *msg_array;          /*Pre-allocated array*/
    uint16_t              msg_num;             /*Number of outstanding requests */
    TF_NET_MUTUX          mutex;               /*Mutex protecting the transport structure */
    tf_net_conn_list      msg_list;            /*Message list head */
    tf_net_conn_list      free_req_list;       /*Free request block list */
    uint8_t               connected;           /*Transport state */
}TF_NET_CONN;

//同步并发数目需要评估
#define TF_CTRL_NET_CONN_MSG_NUM     (1024)
#define FK_NET_CONN_MSG_NUM      (1024)

#define TF_NET_CFG_ITEM_NUM          512

//通用状态标记
typedef enum
{
    NET_COM_PARAM_INIT = 0,
    NET_COM_PARAM_IS_SET,
    NET_COM_PARAM_IS_UNSET,
    NET_COM_PARAM_INVALID = 0xff,

}TF_NET_COM_PARAM;

typedef enum
{
    NET_ALARM_PARA_TYPE_TFID       = 0x00000001,
    NET_ALARM_PARA_TYPE_AUTHID     = 0x00000002,
    NET_ALARM_PARA_TYPE_ONU_SN     = 0x00000004,
    NET_ALARM_PARA_TYPE_ONU_PWD    = 0x00000008,
    NET_ALARM_PARA_TYPE_ROGUE_SN   = 0x00000010,
    NET_ALARM_PARA_TYPE_SLOT       = 0x00000020,
    
}TF_NET_ALARM_PARA_TYPE;

typedef enum
{
    NET_ALARM_STATE_CLEAR,
    NET_ALARM_STATE_REPORT,
    NET_ALARM_STATE_NO_CHANGE,
}TF_NET_ALARM_STATE;

typedef enum
{

    NET_CMD_ALARM_ID_NUM_OF,
}TF_NET_CMD_ALARM_ID;

typedef struct
{
    TF_NET_CMD_ALARM_ID  alarm_id;
    TF_NET_ALARM_STATE   alarm_state;
    int                  para_bit_map;
    uint8_t              slot_id;
    uint8_t              port_id;
    uint8_t              node_id;
}TF_NET_ALARM_INFO;

//配置表项种类
typedef enum
{
    TF_NET_CFG_LINE_PROFILE,
    
    TF_NET_CFG_TYPE_NUM_OF,
    
}TF_TF_NET_CFG_TYPE;

typedef struct
{
    uint32_t total_num;
    uint32_t success_num;
}
NIF_RSL_NUM;

typedef struct
{
    uint32_t mem_load;
    uint32_t cpu_load;
}NIF_LOAD_INFO;

unsigned int 
tf_net_rec_packet_fix(int fd, void *pdata, unsigned int data_len);

unsigned int 
tf_net_rec_packet_fix_peek(int fd, void *pdata, unsigned int data_len);

unsigned int 
tf_net_send_packet(int fd, struct sockaddr_in *p_remote_addr, TF_NET_MSG *pnet_msg);

unsigned int 
tf_net_send_alone_packet(int fd, TF_NET_MSG *pnet_msg);

unsigned int 
tf_net_trbuf_get(int fd, int *sdb_size, int *rdb_size);

unsigned int 
tf_net_trbuf_set(int fd, int sdb_size, int rdb_size);

unsigned int 
tf_net_setnonblock(int fd);

unsigned int 
tf_net_init(struct sockaddr_in *psin, int *p_fd);

unsigned int 
tf_net_msg_verify(TF_NET_MSG *pmsg);

TF_NET_MSG *
tf_net_msg_pack( 
                        TF_NET_BLK_POOL           *p_mem_pool,
                        const unsigned long long    vif_info,
                        const TF_NET_MSG_CMD       cmd_info, 
                        const void                  *pvar, 
                        const unsigned int          pvar_len,
                        const unsigned int          param_len);

unsigned int 
tf_net_vif2phy(unsigned long long ulong, TF_NET_PHY_INFO *p_phy);

unsigned int 
tf_net_phy2vif(TF_NET_PHY_INFO *p_phy, unsigned long long *p_ulong);

unsigned int 
tf_net_tf_ni2vif_ullong(  
                                unsigned int        slot_id, 
                                unsigned int        tf_ni,
                                unsigned long long  *p_ullong);
                                             
unsigned int 
tf_net_tf_onu2vif_ullong(
                                unsigned int       slot_id, 
                                unsigned int       tf_ni ,
                                unsigned int       onu_id,
                                unsigned long long *p_ullong);
                                     
unsigned int 
tf_net_tf_alloc2vif_ullong( 
                                    unsigned int       slot_id, 
                                    unsigned int       tf_ni ,
                                    unsigned int       alloc_id,
                                    unsigned long long *p_ullong);

unsigned int 
tf_net_tf_gemport2vif_ullong( 
                                    unsigned int       slot_id, 
                                    unsigned int       tf_ni ,
                                    unsigned int       gemport_id,
                                    unsigned long long *p_ullong);

unsigned int 
tf_net_tf_vlan2vif_ullong( 
                                unsigned int       slot_id, 
                                unsigned int       tf_ni ,
                                unsigned int       vlan_id,
                                unsigned long long *p_ullong);

unsigned int 
tf_net_tf_flow2vif_ullong( 
                                unsigned int       slot_id, 
                                unsigned int       tf_ni ,
                                unsigned int       flow_id,
                                unsigned long long *p_ullong);

unsigned int 
tf_net_tf_gpio2vif_ullong( 
                                unsigned int       slot_id, 
                                unsigned int       tf_ni ,
                                unsigned int       gpio_id,
                                unsigned long long *p_ullong);
                                            
unsigned int 
tf_net_tf_board2vif_ullong( 
                                unsigned int       slot_id, 
                                unsigned long long *p_ullong);

unsigned int
tf_net_msg_need_uncompress(TF_NET_MSG* p_msg);

unsigned int
tf_net_msg_compress_process(
                                TF_NET_MSG*        p_msg,
                                const void*         p_var, 
                                const unsigned int  var_len,
                                unsigned int*       p_out_len);

unsigned int
tf_net_msg_uncompress_process(TF_NET_BLK_POOL* p_mem_pool, TF_NET_MSG** p_msg);

unsigned int 
tf_net_conn_msg_syn_ack_process(TF_NET_CONN *pconn, TF_NET_MSG *pmsg);

TF_NET_CONN_MSG* 
tf_net_conn_msg_get_free(tf_net_conn_list *plist);

void 
tf_net_conn_msg_free(  TF_NET_CONN_MSG  *pcmsg, 
                            tf_net_conn_list *plist);

                                        
#endif

