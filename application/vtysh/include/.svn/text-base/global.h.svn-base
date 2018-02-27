#ifndef _GLOBAL_H
#define _GLOBAL_H


typedef int (*excute_func)(struct vty *vty, struct cmd_element *cmd, int argc, const char **argv);
typedef int (*execute_func_to_all_daemon)(struct vty *vty, const char *line);

typedef int (*config_write)(struct vty *vty);

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define RT_OK (0)
#define RT_ERR (-1)
#define MEM_ERR (-2)
#define PARAMETER_ERROR (-3)
#define TIME_OUT_ERR (-4)

#define HOST_NAME_DEFAULT "\r\nOLT"
#define PASSWORD_DEFAULT "123456"


typedef unsigned char bool;

#if 0
#ifndef HAVE_TYPE
typedef char int8;
typedef unsigned char uint8;

typedef unsigned short uint16;

typedef unsigned long ulong32;

typedef int int32;
typedef unsigned int uint32;
#endif
#endif



enum {
    LOG_MODULE_DEBUG,
    LOG_MODULE_END
};


#define TIME_1MS 1000
#define TIME_10MS 10000
#define TIME_100MS 100000   



#endif
















