/*****************************************************************************
  Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文件名:  ipc_ker.h
  作者:jh.liang
  版本:1.0.0          
  日期:2014/12
  描述: 此文件定义IPC模块主程序

*****************************************************************************/



#ifndef IPC_KER_H
#define IPC_KER_H

#include "ipc_public.h"
#define IPC_KER_MSG_MAX_LENGTH (IPC_APP_MSG_MAX_LENGTH+sizeof(IPC_HEAD))

typedef struct tag_SND_BLOCK_MSG
{
    struct tag_SND_BLOCK_MSG * next;
    struct sockaddr_un dstaddr;
    USHORT datalen; 
    char pData[0];
}SND_BLOCK_MSG;

/*定义IPC模块使用的全局信息,*/
typedef struct 
{
    char                KerSoPath[IPC_SOCKET_PATH_MAX_LENGTH+1];
    struct sockaddr_un  KerSoAddr;
    int                 KerSockFd;
    char 				RecDataBuf[IPC_KER_MSG_MAX_LENGTH]; 
    char 				SndDataBuf[IPC_KER_MSG_MAX_LENGTH]; 
    int 				blockmsgcount;/*当前被阻塞的消息数*/
}IPC_KER_CTL;

/*定义IPC管理各个子模块的数据结构*/
typedef struct
{
    BOOL       IsReged; 
    char       NameMo[IPC_MODULE_NAME_MAX_LENGTH];
    struct sockaddr_un AckSoAddr;
    struct sockaddr_un CmdSoAddr;
    int sndsockfd;
    int sndwouldblock;/*发送阻塞标志*/
    SND_BLOCK_MSG *BlockMsgHead; /*因发送阻塞而加入到队列的消息*/
    SND_BLOCK_MSG *BlockMsgTail;   /*因发送阻塞而加入到队列的消息*/
    int retry;
}IPC_KER_MO_CTL;

/*定义订阅事件的模块*/
typedef struct  IpcEventList
{
    struct  IpcEventList  *pNext;
    struct  IpcEventList   *pPre;
    UCHAR    ucMo;  //模块号
}IPC_KER_EVENT_LIST;

#endif //IPC_KER_H

