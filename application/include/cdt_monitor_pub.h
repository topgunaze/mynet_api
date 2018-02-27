/**************************************************************
 * 文件名称:  tf_monitor_pub.h
 * 作           者:  steven.tian
 * 日           期:  2015.07.07
 * 文件描述:  monitor hardware event header
 * 版           本:  V1.00
 * 修改历史:
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/
#ifndef _TF_MONITOR_PUB_H
#define _TF_MONITOR_PUB_H

#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define TELL_MONITOR(server) do {\
                        pid_t ppid = getppid();\
                        char  buf[128] = {0};\
                        FILE* fp = NULL;\
                        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", ppid);\
                        fp = fopen(buf, "r");\
                        memset(buf, 0, sizeof(buf));\
                        if (fp != NULL)\
                        {\
                            if (fgets(buf, sizeof(buf), fp) != NULL && strstr(buf, "monitor"))\
                                kill(ppid, SIGRTMIN);\
                            fclose(fp);\
                            printf("init %s done\n",server);\
                        }\
                    }while(0)

#define REQ_REBOOT() do {\
                        pid_t ppid = getppid();\
                        char  buf[128] = {0};\
                        FILE* fp = NULL;\
                        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", ppid);\
                        fp = fopen(buf, "r");\
                        memset(buf, 0, sizeof(buf));\
                        if (fp != NULL)\
                        {\
                            if (fgets(buf, sizeof(buf), fp) != NULL && strstr(buf, "monitor"))\
                                kill(ppid, SIGUSR2);\
                            fclose(fp);\
                        }\
                    }while(0)

#define REQ_REBOOT_BY_PPID(ppid) do {\
                        char  buf[128] = {0};\
                        FILE* fp = NULL;\
                        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", ppid);\
                        fp = fopen(buf, "r");\
                        memset(buf, 0, sizeof(buf));\
                        if (fp != NULL)\
                        {\
                            if (fgets(buf, sizeof(buf), fp) != NULL && strstr(buf, "monitor"))\
                                kill(ppid, SIGUSR2);\
                            fclose(fp);\
                        }\
                    }while(0)

#define START_CFG_RESTORE(pid)  kill(pid, SIGRTMIN)

#define  TF_MAX_PORTS  16
#define  GE_MAX_PORTS   4
#define  XGE_MAX_PORTS  2
#define  XnGE_MAX_PORTS (GE_MAX_PORTS+XGE_MAX_PORTS)
#define  MAX_CARDS_XGE  1
#define  PWR_MAX_CARDS  2
#define  TF_MAX_CHIPS  1

#define ALL_TF_PORT_LIST(max) (~(0xFFFFFFFF << (max)))
#define ALL_TF_TF_PORT_LIST     ALL_TF_PORT_LIST(TF_MAX_PORTS)
#define ALL_TF_GE_PORT_LIST      ALL_TF_PORT_LIST(GE_MAX_PORTS)
#define ALL_TF_XGE_CARD_LIST    ALL_TF_PORT_LIST(MAX_CARDS_XGE)
#define ALL_TF_PWR_CARD_LIST     ALL_TF_PORT_LIST(PWR_MAX_CARDS)

#define IS_TF_PORT_TRUE(list, port) ((list) & (1 << (port)))

#define TF_PORT_TF_LIST_ADD(list, port) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_TF << 24 ) | (1 << (port))))
#define TF_PORT_GE_LIST_ADD(list, port) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_GE << 24 ) | (1 << (port))))
#define TF_CARD_XGE_LIST_ADD(list, card) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_XGE << 24 ) | (1 << (card))))
#define TF_CARD_PWR_LIST_ADD(list, card) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_PWR << 24 ) | (1 << (card))))
#define TF_DEV_LIST_ADD(list, devId) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_DEV << 24 ) | (1 << (devId))))

#define ALL_TF_PORT_TF_LIST_ADD(list) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_TF << 24 ) | ALL_TF_TF_PORT_LIST))
#define ALL_TF_PORT_GE_LIST_ADD(list) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_GE << 24 ) | ALL_TF_GE_PORT_LIST))
#define ALL_TF_CARD_XGE_LIST_ADD(list) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_XGE << 24 ) | ALL_TF_XGE_CARD_LIST))
#define ALL_TF_CARD_PWR_LIST_ADD(list) ((list) = (((list) & 0x00ffffff) | (LIST_TYPE_PWR << 24 ) | ALL_TF_PWR_CARD_LIST))

#define IS_TF_TF_PORT(list, port) ((((list) & 0xff000000) == (LIST_TYPE_TF << 24)) && ((list) & (1 << (port))))
#define IS_TF_GE_PORT(list, port) ((((list) & 0xff000000) == (LIST_TYPE_GE << 24)) && ((list) & (1 << (port))))
#define IS_TF_XGE_CARD(list, card) ((((list) & 0xff000000) == (LIST_TYPE_XGE << 24)) && ((list) & (1 << (card))))
#define IS_TF_PWR_CARD(list, card) ((((list) & 0xff000000) == (LIST_TYPE_PWR << 24)) && ((list) & (1 << (card))))
#define IS_TF_DEV(list, dev) ((((list) & 0xff000000) == (LIST_TYPE_DEV << 24)) && ((list) & (1 << (dev))))

#define IS_TF_TF_PORT_LIST(list) (((list) & 0xff000000) == (LIST_TYPE_TF << 24))
#define IS_TF_GE_PORT_LIST(list) (((list) & 0xff000000) == (LIST_TYPE_GE << 24))
#define IS_TF_XGE_CARD_LIST(list) (((list) & 0xff000000) == (LIST_TYPE_XGE << 24))
#define IS_TF_PWR_CARD_LIST(list) (((list) & 0xff000000) == (LIST_TYPE_PWR << 24))
#define IS_TF_DEV_LIST(list) (((list) & 0xff000000) == (LIST_TYPE_DEV << 24))

#define TF_ITER_PORT_LIST(list, port, total)	\
    for ((port) = 0; (port) < total; (port)++) \
        if (IS_PORT_TRUE((list), (port)))

#define ITER_SFP_GE_PORT_LIST(list, port) TF_ITER_PORT_LIST((list), (port), GE_MAX_PORTS)

#define LIST_EXTRACT(list, port, newfiled) (((list) & (1 << (port))) ? (newfiled) : 0)
#define IS_FILED_TRUE(com, filed) ((com) & (filed))
#define IS_PORT_TRUE(list, port) ((list) & (1 << (port)))


#define VENDOR_NAME_MAX_LEN 16
#define PRODUCT_NAME_MAX_LEN 16
#define SERIAL_NUMBER_MAX_LEN 16
#define VENDOR_OUI_MAX_LEN  3
#define VENDOR_REVISION_MAX_LEN  4
#define DATA_STR_MAX_LEN 11
#define MAX_FANS       4
#define HW_VERSION_LEN 8

enum
{
    CPU_LOAD_ONE     = 0,
    CPU_LOAD_FIVE    = 1,
    CPU_LOAD_QUARTER = 2,
    CPU_LOAD_MAX_NUM
};

#ifndef _SOC_DRV_GPIO_H
enum
{
    ALARM_CLEAR  = 0,
    ALARM_LEVEL0,
    ALARM_LEVEL1,
    ALARM_LEVEL2,
    ALARM_MAX_SUPPORT
};

enum
{
    KEY_NONE     = 0,
    KEY_PRESSING = 1,
    KEY_SHORT    = 2,
    KEY_LONG     = 3
};
#endif

typedef struct
{
    unsigned char exist;
    unsigned char los;
    unsigned char dis;
    unsigned char good;
    unsigned char change;
}status_t;

typedef struct
{
    int  temp;/* 0.1degree */
    int  vcc;/* 0.01V */
    int  bias;/* 1mA */
    int  txPower;/* 0.01dBm */
    int  rxPower;/* 0.01dBm */
}portDdmInfo_t;

typedef struct
{
    char    name[VENDOR_NAME_MAX_LEN + 1];   /*ASCII*/
    uint8_t oui[VENDOR_OUI_MAX_LEN];
    char    pn[PRODUCT_NAME_MAX_LEN + 1];     /* ASCII */
    char    rev[VENDOR_REVISION_MAX_LEN + 1];     /* ASCII */
    char    sn[SERIAL_NUMBER_MAX_LEN + 1];     /* ASCII */
    char    date[DATA_STR_MAX_LEN];   /* 2014/12/31 */
}portBaseInfo_t;

/* tf port event data structure */
typedef struct tfPortEvt_st
{
    unsigned int  port;
    unsigned char linkUpDown;
    unsigned char loss;
}tfPortEvt_t;

typedef struct alarmEvent_st
{
    int32_t  type;
    int32_t  status;
    int32_t  cur;
    time_t   time;
}alarmEvent_t;

typedef struct portStatus_st
{
    unsigned int  port;
    status_t      status;
}portStatus_t;

typedef struct portInfo_st
{
    status_t       status;
    portDdmInfo_t  ddm;
    portBaseInfo_t base;
}portInfo_t;

typedef struct
{
    unsigned short fanSpeed[MAX_FANS]; /* RPM */
    int            temp;/* 0.1degree */
    unsigned int   memTotal;
    unsigned int   memFree;
    unsigned int   memUsed;/* 0.01 */
    unsigned int   cpuLoad;/* 0.01 */
    char           hwVer[HW_VERSION_LEN + 1];/* Vx.0 */
}sysHwInfo_t;

typedef struct
{
    int32_t  cpuUsed; /* 0.01 */
    float    cpuLoad[CPU_LOAD_MAX_NUM];
}sysCpuInfo_t;

typedef struct ipcDevStatus_st
{
    uint32_t exist;
    uint32_t dis;
    uint32_t good;
}ipcDevStatus_t;

typedef struct ipcRtcData_st
{
    int  year;
    int  month;
    int  day;
    int  hour;
    int  min;
    int  sec;
}ipcRtcData_t;

typedef struct ipcPortData_st
{
    unsigned int portlist;
    int  data;
}ipcPortData_t;

typedef struct onuIgmpModeMsg_st
{
    unsigned short tfId;
    unsigned short onuId;
}onuIgmpModeMsg_t;

typedef struct igmpOnuIdMsg_st
{
    unsigned short tfId;
    unsigned short llid;
}igmpOnuIdMsg_t;


#endif

