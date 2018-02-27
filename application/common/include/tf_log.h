/**************************************************************
 * 文件名称:  tf_log.h
 * 作           者:  steven.tian
 * 日           期:  2015.09.22
 * 文件描述:  system log interface header
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/


#ifndef _TF_LOG_H
#define _TF_LOG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

/* Heap manner, resulting in the output of the information is confusing */
#undef BUF_USE_HEAP
#define BUF_USE_STACK 1

/* For factory version */
#undef __FACTORY_VER__

#ifndef __FACTORY_VER__ 
#define TF_LOG_PATH "/mnt/log/"
#define TF_TMP_LOG_PATH "/var/log/"
#else
#define TF_LOG_PATH "/var/log/"
#define TF_TMP_LOG_PATH TF_LOG_PATH
#endif
#define TF_LOG_CFG "/etc/tf_log.cfg"
#define TF_OPER_LOG_FILE TF_LOG_PATH "oper.log"
#define TF_ALARM_LOG_FILE TF_LOG_PATH "alarm.log"
#define TF_OPER_LOG_TMP_FILE TF_TMP_LOG_PATH "oper.log"
#define TF_LOG_FILE_MAX_LEN 32
enum
{
    TFLOG_NOLOG_INDEX        = 0,
    TFLOG_FILE_INDEX         = 1,
    TFLOG_SYSLOG_INDEX       = 2,
    TFLOG_CLI_INDEX          = 3,
    TFLOG_STDERR_INDEX       = 4,
    TFLOG_MAX_INDEX
};

enum
{
    TFLOG_NOLOG    = 1 << TFLOG_NOLOG_INDEX,
    TFLOG_FILE	    = 1 << TFLOG_FILE_INDEX,
    TFLOG_SYSLOG   = 1 << TFLOG_SYSLOG_INDEX,
    TFLOG_CLI      = 1 << TFLOG_CLI_INDEX,
    TFLOG_STDERR   = 1 << TFLOG_STDERR_INDEX,
};

enum
{
    TF_LOG_TYPE_OPER  = 0,
    TF_LOG_TYPE_ALARM = 1,
    TF_LOG_TYPE_DEBUG = 2,
    TF_LOG_TYPE_NUM
};

#define TF_LOG_PRIO_MASK(p) (((p) > LOG_DEBUG) ? 0 : (1 << (p)))
#define TF_LOG_PRIO_MASK_HL(p) (((p) > LOG_DEBUG) ? (~(0xFFFFUL << (LOG_DEBUG + 1))) : (~(0xFFFFUL << ((p) + 1))))
#define TF_LOG_PRIO_MASK_DEF_DEBUG TF_LOG_PRIO_MASK_HL(LOG_DEBUG)
#define TF_LOG_PRIO_MASK_DEF_ALARM TF_LOG_PRIO_MASK_HL(LOG_NOTICE)


#define CLI_MSG_LEN               (1 << 10)
#if 0
#define TF_LOG_FILE_SIZE  (1 << 20) /* 1MBytes */
#define TF_LOG_LINE_LEN   256
#else
#define TF_LOG_FILE_SIZE  (1 << 14) /* 1MBytes */
#define TF_LOG_LINE_LEN   100
#endif

#define TF_ALARM_FAULT_ID(mod, sub, type, id) ((((mod) & 0xF)) << 24 | ((sub) & 0xF) << 20 | 1 << 16 | ((type) & 0xF) << 12 | ((id) & 0xFFF))
#define TF_ALARM_RECOVERY_ID(mod, sub, type, id) ((((mod) & 0xF)) << 24 | ((sub) & 0xF) << 20 | 1 << 16 | ((type) & 0xF) << 12 | ((id) & 0xFFF))
#define TF_EVENT_RUNNING_ID(mod, sub, type, id) ((((mod) & 0xF)) << 24 | ((sub) & 0xF) << 20 | 1 << 16 | ((type) & 0xF) << 12 | ((id) & 0xFFF))
#define TF_EVENT_SECURITY_ID(mod, sub, type, id) ((((mod) & 0xF)) << 24 | ((sub) & 0xF) << 20 | 1 << 16 | ((type) & 0xF) << 12 | ((id) & 0xFFF))
#define ALARM_FAULT_VAL_NAME(name) alarm_fault_##name


#define ZMALLOC(type, len) malloc((len))
#define ZFREE(type, ptr) do {\
                                if(ptr != NULL)\
                                {\
                                    free(ptr);\
                                    ptr = NULL;\
                                }\
                            }while(0)

enum
{
    TF_ALARM_EVENT_TYPE_COMMUNICATION = 0,
    TF_ALARM_EVENT_TYPE_SERVICE       = 1,
    TF_ALARM_EVENT_TYPE_PROCESS       = 2,
    TF_ALARM_EVENT_TYPE_EQUIPMENT     = 3,
    TF_ALARM_EVENT_TYPE_ENVIRONMENT   = 4,
    TF_ALARM_EVENT_TYPE_UNKNOWN
};


typedef enum 
{
  TFLOG_NONE,
  TFLOG_DEFAULT,
  TFLOG_ZEBRA,
  TFLOG_RIP,
  TFLOG_BGP,
  TFLOG_OSPF,
  TFLOG_RIPNG,  
  TFLOG_OSPF6,
  TFLOG_MASC
} tflog_proto_t;

typedef struct
{
    int   debugEn;  /* enable debug output */
}debugCtrl_t;

typedef struct
{
    int          validCnt;
    unsigned char termOutEn;
    debugCtrl_t *ctrl;
}tflog_debug_t;

typedef struct
{
    int fd;
    dev_t dev;
    ino_t ino;
    char filename[TF_LOG_FILE_MAX_LEN];
    unsigned long fileSizeLimit;
    pthread_mutex_t lock_mutex;
    char filelock[TF_LOG_FILE_MAX_LEN];
    int  lock_fd;
    unsigned int syslogPrioMask;
    unsigned int termPrioMask;
    unsigned int recordPrioMask;
}tflog_catogory_t;

struct tflog 
{
    const char *ident;
    int flags;
    int stat;
    int facility;		/* as per syslog facility */
    tflog_debug_t debug;
    int (*termOut)(char *);
    tflog_catogory_t catogory[TF_LOG_TYPE_NUM];
};

typedef struct alarm_event_st
{
    unsigned long id; /* [31:24]: module [23:20]: class [19:16]: type [15:12]: submod [11:0]: id */
    unsigned char level; /* 0:cirtical 1:major 2:minor 3:warning */
    unsigned char defLevel;
    unsigned char flags; /* [0]:CLI output */
    int (*paramfunc)(char *, int , char **);
    char         *name;
    char         *desc;
    char         *cause;
    char         *advice;
    struct alarm_event_st *next;
}alarm_event_t;

#if 000 /* Closed by keith.gong Conflict with libvtysh.a 20151019. */
/* Message structure. */
struct message
{
  int key;
  char *str;
};
#endif


/* DEFSH for vtysh. */
#define DEFALARMFAULTFUNC(name, alarmid, level, flag, nameStr, descStr, causeStr, advStr) \
  int alarm_fault_##name##_func(char *buf, int len, char **argv);\
  alarm_event_t ALARM_FAULT_VAL_NAME(name) = \
  { \
    alarmid, \
    level, \
    level, \
    flag,  \
    alarm_fault_##name##_func,  \
    nameStr, \
    descStr, \
    causeStr, \
    advStr ,\
    NULL \
  };\
  int alarm_fault_##name##_func(char *buf, int len, char **argv)


/* Default logging strucutre. */
extern struct tflog *tflog_default;

/* Open tflog function */
struct tflog *opentflog (const char *, int, int, int, int, int (*)(char *));

/* Close tflog function. */
void closetflog (struct tflog *cl);

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a,b) __attribute__ ((__format__ (__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a,b)
#endif /* __GNUC__ */

/* Generic function for tflog. */
void tflog (struct tflog *cl, int priority, const char *format, ...) PRINTF_ATTRIBUTE(3, 4);

/* Handy tflog functions. */
void tflog_oper_crit (const char *format, ...)PRINTF_ATTRIBUTE(1, 2);
void tflog_oper_err (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_oper_warn (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_oper_info (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_oper_notice (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_oper (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_alarm_crit (const char *format, ...)PRINTF_ATTRIBUTE(1, 2);
void tflog_alarm_err (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_alarm_warn (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_alarm_info (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_alarm_notice (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_crit (const char *format, ...)PRINTF_ATTRIBUTE(1, 2);
void tflog_err (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_warn (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_info (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_notice (const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
void tflog_debug (int mod, const char *format, ...) PRINTF_ATTRIBUTE(2, 3);

/* Set tflog flags. */
void tflog_set_flag (struct tflog *cl, int flags);
void tflog_reset_flag (struct tflog *cl, int flags);

/* Set tflog filename. */
int tflog_set_file (struct tflog *cl, int flags, char *filename);
int tflog_reset_file (struct tflog *cl);

/* Rotate log. */
int tflog_rotate (struct tflog *cl);

#if 000 /* Closed by keith.gong Conflict with libvtysh.a 20151019. */
/* For hackey massage lookup and check */
#define LOOKUP(x, y) mes_lookup(x, x ## _max, y)

char *lookup (struct message *, int);
char *mes_lookup (struct message *meslist, int max, int index);
#endif

extern const char *tflog_priority[];

int alarm_event_reg(alarm_event_t *alarm_event);
void tflog_debug_mod_enable(int mod, int enable);
void tflog_debug_term_enable(int enable);
int tflog_debug_term_state_get(void);
int tflog_debug_mod_param_get(int mod, int *debugEn);
int tflog_alarm_priority_set(int prio);
int tflog_alarm_priority_get(void);
int tflog_syslog_priority_set(int prio);
int tflog_syslog_priority_get(void);


#endif /* _ZEBRA_LOG_H */
