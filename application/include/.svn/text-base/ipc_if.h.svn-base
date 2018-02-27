/******************************************************************************
Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文 件 名   : ipc_if.h
  作    者   : jh.liang
  版 本 号   : 1.0.0 
  生成日期   : 2015年3月13日
  功能描述   : IPC模块相关变量及宏定义
******************************************************************************/



#ifndef IPC_IF_H
#define IPC_IF_H

#include "ipc_public.h"
#include "pthread.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "errno.h"
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>


#ifdef DEBUG
#define ipc_debug_printf(format, ...) \
do{ \
  fprintf(stderr, format, ##__VA_ARGS__); \
}while(0)
#else
#define ipc_debug_printf(format, ...) 
#endif

extern pthread_mutex_t gModInitMutex ;
extern int modInit;
extern pthread_mutex_t gSdkInitMutex ;
extern int sdkInited;

/*最大消息长度*/
#define  IPC_APP_MSG_MAX_LENGTH  8092
/*定义最大模块名长度*/
#define  IPC_MODULE_NAME_MAX_LENGTH   (4*32-1)
/*定义socket路径最大长度*/
#define IPC_SOCKET_PATH_MAX_LENGTH    (4*32-1)
#define MAX_ERROR_MSG_LEN 256

/*定义IPC头部，一共占用24个字节*/
typedef struct
{
    USHORT     usMagic;    
    USHORT     Ver:4,      
               RCode:12;   
    USHORT     usSn;       
    USHORT     usDataLen;  
    UCHAR      ucSrcMo;   
    UCHAR      ucDstMo;    
    UCHAR      ucMsgType;  
    UCHAR      SynFlag:2,  
               NewIpcH:2;  
                    
    ULONG      ulRes0;     
    ULONG      ulRes1;
    ULONG      ulRes2;
}IPC_HEAD;

typedef struct 
{
    int signum;
    char module_name[32];
    siginfo_t info;
    ucontext_t ucontext;
}module_killed_info;

#define IPC_MSG_MAX_LENGTH (IPC_APP_MSG_MAX_LENGTH+sizeof(IPC_HEAD))

/*定义回调函数格式*/
typedef ULONG (*IPC_MSG_CALLBACK) (char *pMsgOut,ULONG ulMsgLen,UCHAR ucMsgType,UCHAR ucSrcMo);

//定义接口层使用的全局数据信息
typedef struct
{
    ULONG     gInitFlag;
    BOOL      Registered;
    UCHAR     ucSrcMo;   
    char      NameMo[IPC_MODULE_NAME_MAX_LENGTH];
    char      SoAckPath[IPC_SOCKET_PATH_MAX_LENGTH+1]; 
    char      SoCmdPath[IPC_SOCKET_PATH_MAX_LENGTH+1]; 

    struct sockaddr_un AckSoAddr;
    struct sockaddr_un CmdSoAddr;
    struct sockaddr_un IpcSoAddr; 

    int    AckSockFd; 
    int    CmdSockFd; 

    //接收同步响应的缓冲
    char AckRecDataBuf[IPC_MSG_MAX_LENGTH];
    //接收命令消息的缓冲
    char CmdRecDataBuf[IPC_MSG_MAX_LENGTH];

    USHORT  usMsgSn;//发送消息的序列序列号
    IPC_MSG_CALLBACK    pIfCallBack;
}IPC_IF_MODULE_INFO;

//IPC返回值定义
typedef enum
{
    IPC_SUCCESS=0,    
    IPC_TIMEOUT=1,    
    IPC_REC_INVALID_MSG=2, 
    IPC_INVALID_MODULE=3,
    IPC_NOT_INIT=4, 
    IPC_INVALID_HEAD=5, 
    IPC_NULL_PTR=6,
    IPC_MEM_LACK=7,
    IPC_SOCK_CREATE_FAIL=8, 
    IPC_SRC_MO_NAME_INVALID=9,
    IPC_SOCKET_SENDTO_FAIL=10, 
    IPC_CREATE_THREAD_FAIL=11,
    IPC_OBJ_EXIST=12,
    IPC_INVALID_EID=13,
    IPC_REC_MSG_IPC_HEAD=14,
    IPC_CMD_MODULE_LOOP=15,
    IPC_EVENT_NOT_ENGAGED=16,
    IPC_REG_DUP=17, 
    IPC_SEND_BLOCK=18,
    IPC_COMMON_FAIL=1000, 
    
}IPC_IF_RET_CODE;

typedef enum
{
    IPC_MSG_CMD,            
    IPC_MSG_ACK,             
    IPC_MSG_ECHO,            
    IPC_MSG_REG_M,          
    IPC_MSG_REG_M_ACK,      
    IPC_MSG_DISREG_M,        
    IPC_MSG_DISREG_M_ACK,  
    IPC_MSG_NOTIFY,        
    IPC_EVENT_ENGAGE,       
    IPC_EVENT_ENGAGE_ACK,   
    IPC_EVENT_DIS_ENGAGE,    
    IPC_EVENT_DIS_ENGAGE_ACK,
    IPC_EVENT_RELEASE,      
}IPC_MSG_TYPE;

/*申明对外的接口函数*/
void  ipc_if_init();
ULONG ipc_if_reg_module(UCHAR srcMo,char *pNameMo,IPC_MSG_CALLBACK pCallBack);
ULONG ipc_if_disreg_module();
ULONG ipc_if_send_synmsg(USHORT usDstMo,char *pAppMsgSend,USHORT usLenSend,UCHAR ucMsgType,char **pMsgRec,USHORT *pLenRec);
ULONG ipc_if_send_asynmsg(USHORT usDstMo,char *pAppMsgSend,USHORT usLenSend,UCHAR ucMsgType);
ULONG ipc_if_send_ack(UCHAR ucDstMo,char * PRecMsg,short RetCode,char *pAckData,USHORT AckDataLen);
ULONG ipc_if_engage_event(UCHAR EventId);
ULONG ipc_if_disengage_event(UCHAR EventId);
ULONG ipc_if_release_event(UCHAR EventId,char *pDataSend,USHORT usLenSend);
void *ipc_if_alloc(ULONG ulLenth);
ULONG ipc_if_free(void* pToFree);
ULONG ipc_if_preack(char * pRecMsg, char *pSendMsg);
UCHAR ipc_if_get_thismoid();

int ipc_if_get_cmd_result(unsigned short dstModuleID,short MsgID,char *rcvbuf,int buflen,char *cmd,int cmdlen,short *retCode);
int ipc_if_exe_cmd(unsigned short dstModuleID,short MsgID,char *cmddata,int cmdlen,short *retCode);

void * ipc_init_shmem(char *path,int size,int flag,int *shmemid);
int    ipc_del_shmem(void * addr);
int    ipc_init_sem(char *path,int flag,int *semid);
void   ipc_sem_down(int semid);
void   ipc_sem_up(int semid);
int ipc_rm_shmem(int shmid);
int ipc_rmv_sem(int semid);
pthread_t  ipc_pthread_proc_id();


#ifdef DEBUG
      #define debuginfo(fmt, args...)	 printf(fmt, ## args)
#else
      #define debuginfo(fmt, args...)
#endif

#endif  //IPC_IF_H

