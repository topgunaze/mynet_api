/******************************************************************************
Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文 件 名   : syslog.h
  作    者   : jh.liang
  版 本 号   : 1.0.0 
  生成日期   : 2015年3月13日
  功能描述   : IPC模块相关变量及宏定义
******************************************************************************/



#ifndef SYSLOG_H
#define SYSLOG_H

#include <sys/syslog.h>

#if 0
#define		LOG_EMERG	0	          
#define		LOG_ALERT	1	   
#define		LOG_ERR		2	  
#define		LOG_WARNING	3	  
#define		LOG_NOTICE	4	    
#define		LOG_INFO	5	 
#define		LOG_DEBUG	6  
#endif
#define MAX_ERROR_MSG_LEN 256
void log_print(int level,char* exp, ... );

#endif  //SYSLOG_H


