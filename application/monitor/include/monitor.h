/**************************************************************
 * 文件名称:  monitor.h
 * 作           者:  steven.tian
 * 日           期:  2015.05.15
 * 文件描述:  monitor hardware event header
 * 版           本:  V1.00
 * 修改历史:
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/
#ifndef _MONITOR_H
#define _MONITOR_H

#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include "tf_monitor_pub.h"

#include "tf_log.h"
#include "ipc_public.h"

#define PROMPT_LEN     54
#define PROMPT_RESULT_LEN 8

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MONITOR_PRINT
#define MONITOR_PRINT(fmt,arg...)
#endif

#ifndef MONITOR_ERROR
#define MONITOR_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#endif

#ifndef MONITOR_DEBUG
#define MONITOR_DEBUG(fmt,arg...) tflog_debug(0, "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##arg)
#endif

#ifndef MONITOR_CRIT
#define MONITOR_CRIT(fmt,arg...) tflog_crit(fmt, ##arg)
#endif

#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))
#define LOG_SAVED_PATH      "/mnt/log"
#define LOG_SAVED_TMP_FILE  "/mnt/log/log.tar.gz"
#define LOG_MESSAGE_FILE    "/var/log/message*"
#define LOG_SAVED_INTVAL  (5 * 60) /* default:5min */

#define APP_WAIT_TIMEOUT  (3 * 60) /* default: 3min */

enum
{
    MONITOR_NONE_MOD = 0,
    MONITOR_MAX_MOD
};

enum
{
    APP_NONE            = 0,
    APP_REDIRECT        = (1 << 0),
    APP_SHARE_WAIT      = (1 << 1),
    APP_EXCLUSIVE_WAIT  = (1 << 2),
    APP_CLOSE_IO        = (1 << 3),
    APP_RESTART_SELF    = (1 << 4),
    APP_RESTART_SYSTEM  = (1 << 5),
    APP_START_STAGE1    = (1 << 6),
    APP_START_STAGE2    = (1 << 7),
    APP_START_STAGE3    = (1 << 8),
    APP_START_STAGE4    = (1 << 9),
    APP_START_STAGE5    = (1 << 10),
    APP_DISABLE_WTD     = (1 << 11),
    APP_TERM_SILENT     = (1 << 12),
};
#define APP_START_STAGE_ALL (APP_START_STAGE1 | APP_START_STAGE2 | APP_START_STAGE3 | APP_START_STAGE4 | APP_START_STAGE5)
#define IS_VALID_STAGE(s) ((s) & APP_START_STAGE_ALL)

enum
{
    APP_STATUS_INITIAL  = 0,
    APP_STATUS_RUNNING  = 1,
    APP_STATUS_EXIT     = 2,
    APP_STATUS_TIMEOUT  = 3,
    APP_STATUS_ERR_START = 4,
};

enum
{
    APP_EVENT_ID_IPC_KER        = IPC_EVENT_BASE,
    APP_EVENT_ID_TF            = IPC_EVENT_TF_INITED,
    APP_EVENT_ID_SW_IGMP_SN     = IPC_EVENT_SW_IGMP_SN_INITED,
    APP_EVENT_ID_SW_RSTP        = IPC_EVENT_SW_RSTP_INITED,
    APP_EVENT_ID_SW_LACP        = IPC_EVENT_SW_LACP_INITED,
    APP_EVENT_ID_SW_DOT1X       = IPC_EVENT_SW_DOT1X_INITED,
    APP_EVENT_ID_SW_DHCP_SN     = IPC_EVENT_SW_DHCP_SN_INITED,
    APP_EVENT_ID_SW_L2          = IPC_EVENT_SW_L2_INITED,
    APP_EVENT_ID_SW_TRUNK       = IPC_EVENT_SW_TRUNK_INITED,
    APP_EVENT_ID_SW_SDK         = IPC_EVENT_SDK_INITED,
    APP_EVENT_ID_SYSCTRL        = IPC_EVENT_SYSCTRL_INITED,
    APP_EVENT_ID_CLI            = IPC_EVENT_CLI_INITED,
    APP_EVENT_ID_RESV           = IPC_EVENT_MAX,
    APP_EVENT_ID_TERM           ,
    APP_EVENT_ID_SW_QOS_ACL     ,
};

/* reference to Watchdog.h (kernel\linux-3.6.5\include\linux) */
#define	WATCHDOG_IOCTL_BASE	'W'
#define	WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)

typedef struct
{
    const char  *path;
    const char  *name;
    const char  *argStr;
    const unsigned int flag;
    pid_t        pid;
    int          status;
    int          exitStatus;
    unsigned char eventId;
    unsigned char waitFlag;
    const char  *prompt;
    unsigned long timeout;
}appDesc_t;

#endif

