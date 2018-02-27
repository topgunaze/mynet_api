/*
 *
 * keith.gong
 *
 * 2014/03/06 - ?
 *
 */

#ifndef __TF_QUEUE_H__
#define __TF_QUEUE_H__

#include <tf_types.h>

#define TF_QUEUE_NO_WAIT                      (0)
#define TF_QUEUE_WAIT_FOREVER              (-1)


typedef enum
{
    TF_QUEUE_SWITCH_IGMP,
    TF_QUEUE_SWITCH_RSTP,
    TF_QUEUE_RSTP_TIMER,

    TF_QUEUE_NUM,
}tfQueueSubKey;


typedef enum
{
    TF_SW_MSG_ANY = 0,
    TF_SW_MSG_ARP,
    TF_SW_MSG_DHCP,
    TF_SW_MSG_IGMP,
    TF_SW_MSG_RSTP,
    TF_SW_MSG_LACP,
    TF_SW_MSG_DOT1X,
    TF_SW_MSG_PPPOEPLUS,

	TF_SW_MSG_MAX,
}tfSwQueueMsgType;


#define TF_SW_MSG_TEXT_SIZE                1550
#define TF_SW_MSG_MAX_NUM                  30
#define TF_DHCP_PKT_LEN_MAX 900     /*发送给DHCP 模块的消息长度*/
#define TF_DHCP_PKT_LEN_LIMIT 800   /*接收到的DHCP包的最大长度*/
#define TF_DHCP_PKT_LEN_EXPEND 100     /*发送给DHCP 模块的消息长度=pkt_len+TF_DHCP_PKT_LEN_EXPEND*/

typedef struct
{
    long                                    type;
    UINT8                                   srcPort;
    UINT16                                  pktLen;
    UINT16                                  llid;
    UINT16                                  devId;
    UINT16                                  vid;
    UINT16                                  onuId;
    UINT8                                   igmpMode;
}tfSwitchIgmpMsgHeader_t;

typedef struct
{
    long        type;
    INT8        port;
    INT8        trunk;
    UINT8       noVlan;
    UINT16      vid;
    UINT16      len;
    UINT8       data[TF_SW_MSG_TEXT_SIZE];
}tfRstpBpduMsg_t;

typedef struct
{
    tfSwitchIgmpMsgHeader_t                hdr;
    UINT8                                   data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchIgmpMsg_t;


typedef struct
{
    long type;
    UINT8 tfId;
    UINT8 onuId;
    UINT8 llid;
    UINT8 uniId;
    UINT8 srcMac[6];
    UINT8 srcport;
    UINT8 direction;                /*报文方向;client->server/server->client*/
    UINT32 pktLen;  		/*报文长度*/
    UINT16 ipHeadOffset;   /*IP 头的偏移值*/
    UINT16 vid;
    UINT32  dhcpOffset;            /*dhcp报文的偏移值*/
}tfSwitchDhcpMsgHeader_t;

typedef struct
{
    tfSwitchDhcpMsgHeader_t hdr;
    UINT8 data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchDhcpMsg_t;

typedef struct
{
	long                                    type;
	UINT8                                   srcPort;
	UINT16                                  pktLen;
	UINT16                                  vid;
}tfSwitchArpMsgHeader_t;

typedef struct
{
    tfSwitchArpMsgHeader_t hdr;
    UINT8 data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchArpMsg_t;

typedef struct
{
    long        type;       /* TF_SW_MSG_LACP */
    UINT8       srcPort;    /* 入接口索引 */
    UINT16      pktLen;
    UINT16      vid;        /* 实际不涉及 */
}tfSwitchLacpMsgHeader_t;

typedef struct
{
    tfSwitchLacpMsgHeader_t hdr;
    UINT8 data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchLacpMsg_t;

typedef struct
{
    long        type;       /* TF_SW_MSG_DOT1X */
    UINT8       srcPort;    /* 入接口索引 */
    UINT16      pktLen;
    UINT16      vid;        /* 暂时不区分 */
}tfSwitchDot1xMsgHeader_t;

typedef struct
{
    tfSwitchDot1xMsgHeader_t hdr;
    UINT8 data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchDot1xMsg_t;

typedef struct
{
    long        type;       /* TF_SW_MSG_DOT1X */
    UINT8       srcPort;    /* 入接口索引 */
    UINT16      pktLen;
    UINT16      vid;        /* 暂时不区分 */
}tfSwitchPplusMsgHeader_t;

typedef struct
{
    tfSwitchPplusMsgHeader_t hdr;
    UINT8 data[TF_SW_MSG_TEXT_SIZE];
}tfSwitchPplusMsg_t;

INT32 tfQueueCreate (INT32 *queue_id, INT32 subKey, UINT32 queue_size);
INT32 tfQueueGet (INT32 queue_id, long type, void *data, UINT32 size, UINT32 *size_copied, INT32 timeout);
INT32 tfQueuePut (INT32 queue_id, void *data, UINT32 size, INT32 timeout);


#endif

