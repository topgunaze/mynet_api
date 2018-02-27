/**************************************************************
 * 文件名称:  gtfSyslog.h
 * 作           者:  keith.gong
 * 日           期:  2015.09.23
 * 文件描述:  GTF 系统日志
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef __GTF_SYSLOG_H__
#define __GTF_SYSLOG_H__


#include <stdio.h>
#include <time.h>
#include "tf_log.h"


typedef enum
{
    GTF_SYSLOG_MODULE_DRV,
    GTF_SYSLOG_MODULE_HWAL,
    GTF_SYSLOG_MODULE_OMCI,
    GTF_SYSLOG_MODULE_ONUREG,
    GTF_SYSLOG_MODULE_PROFILE,
    GTF_SYSLOG_MODULE_ONUCFG,
    GTF_SYSLOG_MODULE_TFCFG,
    GTF_SYSLOG_MODULE_DEVCTRL,
    GTF_SYSLOG_MODULE_SNMP,
    GTF_SYSLOG_MODULE_ROGUE,
    
    GTF_SYSLOG_MODULE_MAX_NUM,
}GTF_SYSLOG_MODULE_ID_ST;

#if 1
#define DRV_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define DRV_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define DRV_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_DRV, "[DRV ] "fmt, ##arg)

#define HWAL_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define HWAL_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define HWAL_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_HWAL, "[HWAL] "fmt, ##arg)

#define ROGUE_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define ROGUE_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define ROGUE_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_ROGUE, "[ROGUE] "fmt, ##arg)


#define OMCI_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define OMCI_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define OMCI_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_OMCI, "[OMCI] "fmt, ##arg)

#define ONURA_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define ONURA_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define ONURA_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_ONUREG, "[RA ] "fmt, ##arg)

#define PROFILE_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define PROFILE_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define PROFILE_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_PROFILE, "[ROFILE] "fmt, ##arg)

#define ONUCFG_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define ONUCFG_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define ONUCFG_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_ONUCFG, "[OCFG] "fmt, ##arg)

#define TFCFG_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define TFCFG_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define TFCFG_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_TFCFG, "[PCFG] "fmt, ##arg)

#define TFDEVCTRL_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define TFDEVCTRL_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define TFDEVCTRL_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_DEVCTRL, "[DCTL] "fmt, ##arg)

#define SNMP_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define SNMP_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define SNMP_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_SNMP, "[SNMP] "fmt, ##arg)

#define ALARM_PRINT(fmt,arg...) tflog_info(fmt, ##arg)
#define ALARM_ERROR(fmt,arg...) tflog_crit(fmt, ##arg)
#define ALARM_DEBUG(fmt,arg...) tflog_debug(GTF_SYSLOG_MODULE_SNMP, "[ALARM] "fmt, ##arg)

#else
#if 111
#define DRV_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define DRV_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define DRV_DEBUG(fmt,arg...) printf("[DRV ] "fmt"\r\n", ##arg)

#define HWAL_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define HWAL_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define HWAL_DEBUG(fmt,arg...) printf("[HWAL] "fmt"\r\n", ##arg)

#define OMCI_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define OMCI_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define OMCI_DEBUG(fmt,arg...) printf("[OMCI] "fmt"\r\n", ##arg)

#define ONURA_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define ONURA_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define ONURA_DEBUG(fmt,arg...) printf("[RA ] "fmt"\r\n", ##arg)

#define PROFILE_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define PROFILE_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define PROFILE_DEBUG(fmt,arg...) printf("[PROF] "fmt"\r\n", ##arg)

#define ONUCFG_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define ONUCFG_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define ONUCFG_DEBUG(fmt,arg...) printf("[OCFG] "fmt"\r\n", ##arg)

#define TFCFG_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define TFCFG_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define TFCFG_DEBUG(fmt,arg...) printf("[PCFG] "fmt"\r\n", ##arg)

#define TFDEVCTRL_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define TFDEVCTRL_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define TFDEVCTRL_DEBUG(fmt,arg...) printf("[DCTL] "fmt"\r\n", ##arg)

#define SNMP_PRINT(fmt,arg...) printf(fmt"\r\n", ##arg)
#define SNMP_ERROR(fmt,arg...) printf(fmt"\r\n", ##arg)
#define SNMP_DEBUG(fmt,arg...) printf("[SNMP] "fmt, ##arg)

#else

//#define DRV_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define DRV_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define DRV_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

//#define HWAL_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define HWAL_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define HWAL_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

#define OMCI_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
#define OMCI_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
#define OMCI_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

//#define ONURA_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define ONURA_ERROR(fmt,arg...)tflog_err(fmt, ##arg)
//#define ONURA_DEBUG(fmt,arg...)tflog_err(fmt, ##arg)

//#define PROFILE_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define PROFILE_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define PROFILE_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

//#define ONUCFG_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define ONUCFG_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define ONUCFG_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

//#define TFCFG_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define TFCFG_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define TFCFG_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)


//#define TFDEVCTRL_PRINT(fmt,arg...) tflog_err(fmt, ##arg)
//#define TFDEVCTRL_ERROR(fmt,arg...) tflog_err(fmt, ##arg)
//#define TFDEVCTRL_DEBUG(fmt,arg...) tflog_err(fmt, ##arg)

#endif
#endif

#endif /* __GTF_SYSLOG_H__ */


