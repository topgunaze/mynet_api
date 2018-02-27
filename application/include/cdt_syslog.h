/******************************************************************************
Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文 件 名   : tf_syslog.h
  作    者   : jh.liang
  版 本 号   : 1.0.0 
  生成日期   : 2015年3月13日
  功能描述   : IPC模块相关变量及宏定义
******************************************************************************/



#ifndef TF_SYSLOG_H
#define TF_SYSLOG_H

//#include <sys/syslog.h>
#include "tf_log.h"

struct tflog *
opentflog (const char *progname, int mods, int debugEn,int flags, int syslog_facility, int (*termOut)(char *));

#define log_print(level,exp, arg... )                                           \
    do {                                                                        \
        switch(level)                                                           \
        {                                                                       \
            case LOG_CRIT:                                                      \
                tflog_crit("[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg);   \
            break;                                                              \
            case LOG_ERR:                                                       \
                tflog_err("[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg);    \
            break;                                                              \
            case LOG_WARNING:                                                   \
                tflog_warn("[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg);   \
            break;                                                              \
            case LOG_NOTICE:                                                    \
                tflog_notice("[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg); \
            break;                                                              \
            case LOG_INFO:                                                      \
                tflog_info("[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg);   \
            break;                                                              \
            case LOG_DEBUG:                                                     \
                tflog_debug(0, "[%s:%d] "exp, __FUNCTION__, __LINE__, ##arg);  \
            break;                                                              \
            default :                                                           \
                break;                                                          \
        }                                                                       \
    } while(0)       

 #define dump_pkt_print(exp, arg... )                                           \
    do {                                                                        \
                tflog_debug(0, exp, ##arg);  \
        } while(0)                 
#endif  //TF_SYSLOG_H


