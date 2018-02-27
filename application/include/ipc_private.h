/*****************************************************************************
  Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文件名:  ipc_private.h
  作者:jh.liang
  版本:1.0.0          
  日期:2014/12
  描述: 此文件定义IPC模块主程序

*****************************************************************************/


#ifndef IPC_PRIVATE_H
#define IPC_PRIVATE_H

#include "ipc_public.h"

//定义当前的IPC头部版本号
#define IPC_HEAD_CUR_VER    1;
//定义IPC头部的魔术字
#define IPC_HEAD_MAGIC    0xC0D1

//定义接收线程的缺省堆栈大小
#define IPC_REC_CMD_DEFAULT_STACKSIZE 32 * 1024
/*
 * 调度策略
 */
#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2

//定义IPC接收路径
#define IPC_MAIN_REC_SO_PATH    "/tmp/IpcKer"

//定义socket路径起始字符串
#define  IPC_IF_CMD_SO_PATH_PRE  "/tmp/IpcCmd"
#define  IPC_IF_ACK_SO_PATH_PRE  "/tmp/IpcAck"

#define IPC_APP_MEM_LEN_TO_ACT_LEN(l)    (sizeof(IPC_HEAD)+l)
#define IPC_APP_MEM_OFFSET(addr)  (((char*)addr)+sizeof(IPC_HEAD))
#define IPC_APP_MEM_TO_IPC(addr)  (((char*)addr)-sizeof(IPC_HEAD))

#define    IPC_SYN_SELECT_TIMEOUT_SEC    60    


typedef enum
{
    IPC_SYN_MSG,  //同步消息
    IPC_ASYN_MSG //异步消息
}IPC_ASYN_FLAG;

//定义IPC内部返回给IPC接口的同步返回码
typedef enum
{
    IPC_REG_M_ACK, 
    IPC_REG_M_NAK,     
    IPC_DISREG_M_ACK,   
    IPC_DISREG_M_NAK,  
    
    IPC_ENGAGE_EVENT_ACK, 
    IPC_ENGAGE_EVENT_NAK, 

    IPC_DISENGAGE_EVENT_ACK, 
    IPC_DISENGAGE_EVENT_NAK, 
    
}IPC_TO_IF_ACK_NAK;

//定义注册消息格式
typedef struct
{
    UCHAR    ucSrcMo;   
    UCHAR    ucRes0;    
    USHORT   usRes1;
    ULONG     ulRes2;
    char         NameMo[IPC_MODULE_NAME_MAX_LENGTH+1];
    char        SoAckPath[IPC_SOCKET_PATH_MAX_LENGTH+1];
    char        SoCmdPath[IPC_SOCKET_PATH_MAX_LENGTH+1];
}IPC_REG_MODULE_MSG_INFO;

//定义注销模块的消息格式
typedef struct
{
    UCHAR    ucSrcMo;   
    UCHAR    ucRes0;    
    USHORT   usRes1;
    ULONG     ulRes2;
}IPC_DIS_REG_MODULE_MSG_INFO;

//订阅/取消订阅事件的消息格式
typedef struct
{
    UCHAR    ucSrcMo;   
    UCHAR    ucEventId;    
    USHORT   usRes0;
    ULONG     ulRes1;
}IPC_EVENT_CMD_MSG_INFO;

//定义通用响应消息格式
typedef struct
{
    UCHAR    ucSrcMo;   
    UCHAR    ucRetCode; 
    USHORT  usRes0;
    ULONG    ulRes1;
}IPC_COMMON_REG_ACK_INFO;

#endif //IPC_PRIVATE_H

