/*
 * panger123
 *
 * 2014/01-20 - ?
 *
 */ 
#ifndef __TF_SYS_LOG__
#define __TF_SYS_LOG__
#include "tf_types.h"
#define TF_LOG_FILE_SIZE	524288  /* 512KBytes*/

#define TF_LOG_PAGE_LEN	16
#define TF_LOG_LINE_LEN	256
#define TF_LOG_FILE_SHORT_NAME "log.txt"
#define TF_LOG_FILE_FULL_NAME  "/mnt/log.txt"

typedef struct tfSysLogLevel_t
{
	UINT8 onuOnOffLine;
	UINT8 alarmOnuDyingGasp;
	UINT8 alarmUniLoopBack;
}tfSysLogLevel_t;

typedef enum 
{
	TF_LOG_CRIT=0,
	TF_LOG_ALARM,
	TF_LOG_NORMAL,	
	TF_LOG_DEBUG,
}tfLogLevel;

tfSysLogLevel_t sysLogL;

void tfSyslog(tfLogLevel level, INT8 *String, ...);

#endif

