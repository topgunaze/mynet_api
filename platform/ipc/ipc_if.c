/******************************************************************************
Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文 件 名   : ipc_if.c
  作    者   : jh.liang
  版 本 号   : 1.0.0 
  生成日期   : 2015年3月13日
  功能描述   :  此文件定义IPC接口实现函数
******************************************************************************/

#include <sys/socket.h>
#include <sys/un.h>
#include "pthread.h"
#include "ipc_if.h"
#include "ipc_private.h"
#include "stdio.h"
#include "malloc.h"
#include "stdlib.h" 

#include <fcntl.h>
//#include "tfMacCryptExpo.h"


IPC_IF_MODULE_INFO   gIfCtl;
/*因为在模块中都是多线程,这里提供对收发包的计数上锁*/
static pthread_mutex_t gSnMutex= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t gSendSynMutex= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#define LOCK(mutex) pthread_mutex_lock(&mutex) 
#define UNLOCK(mutex) pthread_mutex_unlock(&mutex) 

/*modInit提供初始化标志,gModInitMutex提供对初始化标志的上锁*/
pthread_mutex_t gModInitMutex ;
int modInit = 0;/*进程内部多线程间的同步*/
static pthread_t proc_thread_id = 0;
pthread_mutex_t gSdkInitMutex ;
int sdkInited = 0;/*bcm.user和各子模块间的进程同步*/

//static int semid = -1;
struct sembuf op_down[1]={{0,-1,0}};
struct sembuf op_up[1]={{0,1,0}};

char mips_reg[][10]={
    "0  ($ 0)","AT ($ 1)","v0 ($ 2)","v1 ($ 3)",
    "a0 ($ 4)","a1 ($ 5)","a2 ($ 6)","a3 ($ 7)",
    "t0 ($ 8)", "t1 ($ 9)","t2 ($10)", "t3 ($11)",
    "t4 ($12)", "t5 ($13)","t6 ($14)", "t7 ($15)",
    "s0 ($16)", "s1 ($17)","s2 ($18)", "s3 ($19)",
    "s4 ($20)", "s5 ($21)","s6 ($22)", "s7 ($23)",
    "t8 ($24)", "t9 ($25)", "k0 ($26)","k1 ($27)",
    "gp ($28)", "sp ($29)","fp ($30)", "ra ($31)"
};

//extern  int tfMacLicenseCheck();

static void signal_segv(int signum, siginfo_t* info, void*ptr) {
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
    char buf[256] = {0};
    int  count;
    size_t i;
    module_killed_info module_killed_event;
    ucontext_t *ucontext = (ucontext_t*)ptr;
    /*因为子代理在后台启动会关闭stdout/stdin/stderr,这里重新打开*/
    stderr = fopen("/dev/console","w");
    count = readlink( "/proc/self/exe", buf, 256 );
    buf[ count ] = '\0';
    fprintf(stderr, "path:%s\n",buf);
    fprintf(stderr, "\nSegmentation Fault!\n");
    fprintf(stderr, "info.si_signo = %d\n", signum);
    fprintf(stderr, "info.si_errno = %d\n", info->si_errno);
    fprintf(stderr, "info.si_code = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    fprintf(stderr, "info.si_addr = %p\n", info->si_addr);

    //fprintf(stderr, "EPC=0x%08llx\n",ucontext->uc_mcontext.pc);
    fprintf(stderr, "RA=0x%08llx\n",ucontext->uc_mcontext.gregs[31]);
    for (i = 0; i < NGREG; i++){
          if (i%2 == 0)
          fprintf(stderr, "\n");
          fprintf(stderr, "%s = 0x%08llx          ", mips_reg[i], ucontext->uc_mcontext.gregs[i]);
    }
    fprintf(stderr,"\n");
    module_killed_event.signum = signum;
    strncpy(module_killed_event.module_name,buf,sizeof(module_killed_event.module_name));
    memcpy(&module_killed_event.info,info,sizeof(siginfo_t));
    memcpy(&module_killed_event.ucontext,ucontext,sizeof(ucontext_t));
    ipc_if_release_event(IPC_EVENT_MODULE_KILLED,(char *)&module_killed_event,sizeof(module_killed_info));
    usleep(10);
    exit((0-signum));
}
int setup_sigsegv() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, NULL) < 0) {
        perror("sigaction");
        return 0;
    }

    return 1;
}
#ifndef SIGSEGV_NO_AUTO_INIT
static void __attribute((constructor)) init_sigsegv(void) {
    setup_sigsegv();
}
#endif

static  void ipc_if_ack_sockpath_create()
{
    sprintf(gIfCtl.SoAckPath,"%s_%d",IPC_IF_ACK_SO_PATH_PRE,gIfCtl.ucSrcMo);
}
static  void ipc_if_cmd_sockpath_create()
{
    sprintf(gIfCtl.SoCmdPath,"%s_%d",IPC_IF_CMD_SO_PATH_PRE,gIfCtl.ucSrcMo);
}

static void ipc_if_reccmd_thread()
{
    int Ret;
    int RecLen;
    char *pRecMsg;
    IPC_HEAD *pIpcHead;
    char *pAppData;
    USHORT usAppDataLen;
    UCHAR  ucType;
    UCHAR ucSrcMo;
    int i;
    UCHAR *pDebug;
    fd_set  fds;
    struct timeval timout;

    proc_thread_id = pthread_self();
    for(;;)
    {
        FD_ZERO(&fds);
        FD_SET(gIfCtl.CmdSockFd, &fds);
        timout.tv_sec=IPC_SYN_SELECT_TIMEOUT_SEC;
        timout.tv_usec=0;

        Ret=select(gIfCtl.CmdSockFd + 1, &fds, NULL, NULL, &timout);  
        if(Ret<0)
        {
            printf("ipc_if select error.\r\n");
        }
        
        if(Ret==0)
        {
            continue;
        }
        RecLen=recvfrom(gIfCtl.CmdSockFd,gIfCtl.CmdRecDataBuf,
            IPC_MSG_MAX_LENGTH,0,NULL,NULL);
        if(RecLen<=sizeof(IPC_HEAD))
        {
            continue;
        }
        
        pRecMsg=(char*)malloc(RecLen);
        if(pRecMsg==NULL)
            continue;
        
        memcpy(pRecMsg,gIfCtl.CmdRecDataBuf,RecLen);
        pIpcHead=(IPC_HEAD*)pRecMsg;
        ucType=pIpcHead->ucMsgType;
        ucSrcMo = pIpcHead->ucSrcMo;
        pAppData=IPC_APP_MEM_OFFSET(pRecMsg);
        usAppDataLen=RecLen-sizeof(IPC_HEAD);        
        if(gIfCtl.pIfCallBack)
        {
            gIfCtl.pIfCallBack(pAppData,usAppDataLen,ucType,ucSrcMo);
        }
    }
}

static ULONG  ipc_if_cmdsock_thread()
{
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param sched;
    int errno;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    errno = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setstacksize(&attr, IPC_REC_CMD_DEFAULT_STACKSIZE);
    pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
    errno = pthread_create(&thread, &attr, (void*)ipc_if_reccmd_thread, NULL);
    if (errno)
    {
        return IPC_COMMON_FAIL;
    }
    sched.sched_priority = 50;
    pthread_setschedparam(thread, SCHED_RR, &sched);

    return IPC_SUCCESS;

}

static BOOL ipc_if_snValid(USHORT usOrgSn,char *pMsg)
{
     IPC_HEAD   *pIpcHead;
     
     pIpcHead=(IPC_HEAD  *)pMsg;
     if(usOrgSn==pIpcHead->usSn)
        return TRUE;
     
     return FALSE;
}

static void ipc_if_reset()
{
    close(gIfCtl.AckSockFd);
    close(gIfCtl.CmdSockFd);
    ipc_if_init();
}

void ipc_if_init()
{
#if 0
    if (tfMacLicenseCheck() != 0) {
        printf("License Check error.\r\n");
        exit(0);
    }
#endif        
    memset(&gIfCtl,0,sizeof(IPC_IF_MODULE_INFO));
    gIfCtl.Registered=FALSE;
    gIfCtl.gInitFlag=TRUE;
}

ULONG ipc_if_reg_module(UCHAR srcMo,char *pNameMo,IPC_MSG_CALLBACK pCallBack)
{
    IPC_REG_MODULE_MSG_INFO    *pRegInfo;
    IPC_COMMON_REG_ACK_INFO    *pRegAck;
    char *pMsgRec;
    USHORT usMsgRecLen;
    int Ret;
    int sndbuf = 64*1024;
    if(gIfCtl.gInitFlag!=TRUE)
    {
        return IPC_NOT_INIT;
    }

    if(gIfCtl.Registered==TRUE)
    {
        return IPC_REG_DUP;
    }
    
    if(strlen(pNameMo)>IPC_MODULE_NAME_MAX_LENGTH)
    {
        return IPC_SRC_MO_NAME_INVALID;
    }

    pRegInfo=(IPC_REG_MODULE_MSG_INFO*)ipc_if_alloc(sizeof(IPC_REG_MODULE_MSG_INFO));
    if(pRegInfo==NULL)
        return IPC_MEM_LACK;
    gIfCtl.ucSrcMo=srcMo;
    strcpy(gIfCtl.NameMo,pNameMo);

    ipc_if_ack_sockpath_create(gIfCtl);/*accept ack*/
    ipc_if_cmd_sockpath_create(gIfCtl);/*accept cmd*/

    gIfCtl.IpcSoAddr.sun_family = AF_UNIX;
    sprintf(gIfCtl.IpcSoAddr.sun_path,"%s", IPC_MAIN_REC_SO_PATH);

    gIfCtl.AckSoAddr.sun_family=AF_UNIX;
    sprintf(gIfCtl.AckSoAddr.sun_path,"%s", gIfCtl.SoAckPath);
    if ((gIfCtl.AckSockFd= socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        printf("Create socket Error.\r\n");
        return IPC_SOCK_CREATE_FAIL;
    }
    unlink(gIfCtl.SoAckPath);
    bind(gIfCtl.AckSockFd,(void*)&gIfCtl.AckSoAddr,sizeof(struct sockaddr_un));
    
    pRegInfo->ucSrcMo=srcMo;
    strcpy(pRegInfo->NameMo,pNameMo);
    strcpy(pRegInfo->SoAckPath,gIfCtl.SoAckPath);
    strcpy(pRegInfo->SoCmdPath,gIfCtl.SoCmdPath);
    for(;;)
    {
        Ret=ipc_if_send_synmsg(MODULE_IPC,(char*)pRegInfo,sizeof(IPC_REG_MODULE_MSG_INFO),
            IPC_MSG_REG_M,&pMsgRec,&usMsgRecLen);
        if(IPC_SOCKET_SENDTO_FAIL==Ret)
        {
            sleep(2);
        }
        else
        {
            break;
        }
    }
    
    if(Ret!=IPC_SUCCESS)
    {
        ipc_if_free(pRegInfo);
        return Ret;
    }
    pRegAck=(IPC_COMMON_REG_ACK_INFO*)pMsgRec;
    if((pRegAck->ucSrcMo!=srcMo)||(pRegAck->ucRetCode !=IPC_REG_M_ACK))
    {
        ipc_if_free(pRegInfo);
        ipc_if_free(pMsgRec);
        return IPC_COMMON_FAIL;
    }
    ipc_if_free(pRegInfo);
    ipc_if_free(pMsgRec);

    gIfCtl.CmdSoAddr.sun_family=AF_UNIX;
    sprintf(gIfCtl.CmdSoAddr.sun_path,"%s", gIfCtl.SoCmdPath);
    if ((gIfCtl.CmdSockFd= socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        printf("Create socket Error.\r\n");
        return IPC_SOCK_CREATE_FAIL;
    }
    if (setsockopt(gIfCtl.CmdSockFd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) 
    {
        printf("set SO_SNDBUF Error.\n");
    }    
    if (setsockopt(gIfCtl.CmdSockFd,SOL_SOCKET,SO_RCVBUF,&sndbuf,sizeof(sndbuf)) < 0) {
        printf("set SO_RCVBUF Error.\r\n");
    }    
    unlink(gIfCtl.SoCmdPath);
    bind(gIfCtl.CmdSockFd,(void*)&gIfCtl.CmdSoAddr,sizeof(struct sockaddr_un));
    if(pCallBack == NULL)
    {
        gIfCtl.Registered = TRUE;
        return IPC_SUCCESS;
    }
    
    gIfCtl.pIfCallBack = pCallBack;
    if (IPC_SUCCESS != ipc_if_cmdsock_thread()) {
        printf("Create socket Thread FAIL.\r\n");
        return IPC_CREATE_THREAD_FAIL;
    }

    gIfCtl.Registered = TRUE;
    return IPC_SUCCESS;
    
}

ULONG ipc_if_disreg_module()
{
    IPC_DIS_REG_MODULE_MSG_INFO *pDisRegMsg;
    IPC_COMMON_REG_ACK_INFO    *pAck;
    char *pMsgRec;
    USHORT usMsgRecLen;    
    int Ret;
    UCHAR srcMo = gIfCtl.ucSrcMo;

    if(gIfCtl.Registered==FALSE)
    {
        return IPC_SUCCESS;
    }
    pDisRegMsg=(IPC_DIS_REG_MODULE_MSG_INFO*)ipc_if_alloc(sizeof(IPC_DIS_REG_MODULE_MSG_INFO));
    if(pDisRegMsg==NULL)
        return IPC_MEM_LACK;
    pDisRegMsg->ucSrcMo=srcMo;
    
    Ret=ipc_if_send_synmsg(MODULE_IPC,(char*)pDisRegMsg,sizeof(IPC_DIS_REG_MODULE_MSG_INFO),
        IPC_MSG_DISREG_M,&pMsgRec,&usMsgRecLen);
    if(Ret!=IPC_SUCCESS)
    {
        ipc_if_free(pDisRegMsg);
        return Ret;
    }    
    pAck=(IPC_COMMON_REG_ACK_INFO*)pMsgRec;
    if((pAck->ucSrcMo!=srcMo)||(pAck->ucRetCode !=IPC_DISREG_M_ACK))
    {
        ipc_if_free(pDisRegMsg);
        ipc_if_free(pMsgRec);
        return IPC_COMMON_FAIL;
    }
    ipc_if_reset();
    
    ipc_if_free(pDisRegMsg);
    ipc_if_free(pMsgRec);
    
    return IPC_SUCCESS;

}

ULONG ipc_if_engage_event(UCHAR EventId)
{
    IPC_EVENT_CMD_MSG_INFO *pEngageEvent;
    IPC_COMMON_REG_ACK_INFO    *pAck;
    char *pMsgRec;
    USHORT usMsgRecLen;    
    int Ret;
    UCHAR srcMo = gIfCtl.ucSrcMo;

    pEngageEvent=(IPC_EVENT_CMD_MSG_INFO*)ipc_if_alloc(sizeof(IPC_EVENT_CMD_MSG_INFO));
    if(pEngageEvent==NULL)
        return IPC_MEM_LACK;
    pEngageEvent->ucSrcMo=srcMo;
    pEngageEvent->ucEventId=EventId;
    
    Ret=ipc_if_send_synmsg(MODULE_IPC,(char*)pEngageEvent,sizeof(IPC_EVENT_CMD_MSG_INFO),
        IPC_EVENT_ENGAGE,&pMsgRec,&usMsgRecLen);
    if(Ret!=IPC_SUCCESS)
    {
        ipc_if_free(pEngageEvent);
        return Ret;
    }    
    pAck=(IPC_COMMON_REG_ACK_INFO*)pMsgRec;

    if((pAck->ucSrcMo!=srcMo)||(pAck->ucRetCode !=IPC_ENGAGE_EVENT_ACK))
    {
        ipc_if_free(pEngageEvent);
        ipc_if_free(pMsgRec);
        return IPC_COMMON_FAIL;
    }

    ipc_if_free(pEngageEvent);
    ipc_if_free(pMsgRec);
    
    return IPC_SUCCESS;
    
}

ULONG ipc_if_disengage_event(UCHAR EventId)
{
    IPC_EVENT_CMD_MSG_INFO *pEngageEvent;
    IPC_COMMON_REG_ACK_INFO    *pAck;
    char *pMsgRec;
    USHORT usMsgRecLen;    
    int Ret;
    UCHAR srcMo = gIfCtl.ucSrcMo;

    pEngageEvent=(IPC_EVENT_CMD_MSG_INFO*)ipc_if_alloc(sizeof(IPC_EVENT_CMD_MSG_INFO));
    if(pEngageEvent==NULL)
        return IPC_MEM_LACK;
    pEngageEvent->ucSrcMo=srcMo;
    pEngageEvent->ucEventId=EventId;
    
    Ret=ipc_if_send_synmsg(MODULE_IPC,(char*)pEngageEvent,sizeof(IPC_EVENT_CMD_MSG_INFO),
        IPC_EVENT_DIS_ENGAGE,&pMsgRec,&usMsgRecLen);
    if(Ret!=IPC_SUCCESS)
    {
        ipc_if_free(pEngageEvent);
        return Ret;
    }    
    pAck=(IPC_COMMON_REG_ACK_INFO*)pMsgRec;

    if((pAck->ucSrcMo!=srcMo)||(pAck->ucRetCode !=IPC_DISENGAGE_EVENT_ACK))
    {
        ipc_if_free(pEngageEvent);
        ipc_if_free(pMsgRec);
        return IPC_COMMON_FAIL;
    }

    ipc_if_free(pEngageEvent);
    ipc_if_free(pMsgRec);
    
    return IPC_SUCCESS;
    
}

ULONG ipc_if_release_event(UCHAR EventId,char *pDataSend,USHORT usLenSend)
{
    IPC_EVENT_R_INFO  *pEventMsg;
    int Ret;

    if((pDataSend==NULL)&&(usLenSend!=0))
    {
        return IPC_NULL_PTR;
    }

    pEventMsg=(IPC_EVENT_R_INFO*)ipc_if_alloc(sizeof(IPC_EVENT_R_INFO)+usLenSend);
    if(pEventMsg==NULL)
        return IPC_MEM_LACK;
    
    pEventMsg->EventMsgHead.ucEventId=EventId;
    pEventMsg->EventMsgHead.usLen=usLenSend;

    if (pDataSend)
        memcpy(pEventMsg->data,pDataSend,usLenSend);
    
    Ret=ipc_if_send_asynmsg(MODULE_IPC,(char *)pEventMsg,
        (sizeof(IPC_EVENT_R_INFO)+usLenSend),IPC_EVENT_RELEASE);
    
    ipc_if_free(pEventMsg);

    return Ret;
}

USHORT ipc_get_seqNo()
{
    USHORT sn;

    LOCK(gSnMutex);
            gIfCtl.usMsgSn++;
            sn = gIfCtl.usMsgSn;
    UNLOCK(gSnMutex);
    return sn;
}
/******************************************************************
*因为在模块注册的时候同时注册了一个接收线  *
*程的回调函数,而回调函数中不限制对此函数的 *
*调用,从而此两个线程有并发的可能.                           *
*******************************************************************/
ULONG ipc_if_send_synmsg(USHORT usDstMo,char *pAppMsgSend,USHORT usLenSend
           ,UCHAR ucMsgType,char **pMsgRec,USHORT *pLenRec)
{
    IPC_HEAD  *pIpcHead;
    IPC_HEAD  *pRecIpcHead;
    int Ret;
    int RecLen;
    char *pRecMsg;
    fd_set  fds;
    struct timeval timout;
    int i;
    UCHAR *pData;
    USHORT usSn;
    
    if((pAppMsgSend==NULL)||(pLenRec==NULL))
    {
        return IPC_NULL_PTR;
    }
    
    *pMsgRec=NULL;
    *pLenRec=0;

    pIpcHead=(IPC_HEAD  *)IPC_APP_MEM_TO_IPC(pAppMsgSend);

    if(pIpcHead->usMagic!=IPC_HEAD_MAGIC)
    {
        return IPC_INVALID_HEAD;
    }

    LOCK(gSendSynMutex);

    usSn = ipc_get_seqNo();
    pIpcHead->usSn=usSn;
    pIpcHead->ucMsgType=ucMsgType;
    pIpcHead->ucSrcMo=gIfCtl.ucSrcMo;
    pIpcHead->ucDstMo=usDstMo;
    pIpcHead->usDataLen = usLenSend;
    pIpcHead->SynFlag=IPC_SYN_MSG;
    pIpcHead->RCode=IPC_SUCCESS;

    if (sendto(gIfCtl.AckSockFd, pIpcHead, (sizeof(IPC_HEAD)+usLenSend),
        0, (void*)&gIfCtl.IpcSoAddr, sizeof(struct sockaddr_un)) < 0)
    {
        perror("Send Syn msg FAIL.\r\n");
        UNLOCK(gSendSynMutex); 
        return IPC_SOCKET_SENDTO_FAIL;
    }
    
    for(;;)
    {
        FD_ZERO(&fds);
        FD_SET(gIfCtl.AckSockFd, &fds);
        timout.tv_sec=IPC_SYN_SELECT_TIMEOUT_SEC;
        timout.tv_usec=0;

        Ret=select(gIfCtl.AckSockFd + 1, &fds, NULL, NULL, &timout);    
        if(Ret<0)
        {
            UNLOCK(gSendSynMutex); 
            return IPC_SOCKET_SENDTO_FAIL;
        }
        if(Ret==0)
        {
            printf("ipc if :waiting ACK time out  while send syn msg at line :%d.src=%d dst=%d\n",__LINE__,gIfCtl.ucSrcMo,usDstMo);
            UNLOCK(gSendSynMutex);
            return IPC_TIMEOUT;
        }

        RecLen=recvfrom(gIfCtl.AckSockFd,gIfCtl.AckRecDataBuf,
            IPC_MSG_MAX_LENGTH,0,NULL,NULL);
        if(FALSE==ipc_if_snValid(usSn,gIfCtl.AckRecDataBuf))
        {
            printf("IPC if : Invalid ACK while send syn msg at line :%d.\n",__LINE__);
            continue;
        }
        break;        
    }
    if(RecLen<sizeof(IPC_HEAD))
    {
        printf("IPC if :Receive shortly ACK msg while send syn msg at line :%d.\n",__LINE__);
        UNLOCK(gSendSynMutex);
        return IPC_REC_INVALID_MSG;
    }
    
    pRecIpcHead=(IPC_HEAD*)gIfCtl.AckRecDataBuf;
    if(pRecIpcHead->usMagic!=IPC_HEAD_MAGIC)
    {
        printf("IPC if:Syn sending :Rec Ack with Invalid IPC Head at line:%d\r\n",__LINE__);
        UNLOCK(gSendSynMutex); 
        return IPC_REC_MSG_IPC_HEAD;
    }
    if(pRecIpcHead->RCode!=IPC_SUCCESS)
    {
        printf("IPC if:Syn sending :Rec IPC layer FAIL at line:%d, code=%d .src=%d dst=%d\r\n",__LINE__,pRecIpcHead->RCode,gIfCtl.ucSrcMo,usDstMo);
        UNLOCK(gSendSynMutex); 
        return pRecIpcHead->RCode;
    }

    pRecMsg=(char*)malloc(RecLen);
    if(pRecMsg==NULL)
    {
        UNLOCK(gSendSynMutex);
        return IPC_MEM_LACK;
    }
    
    memcpy(pRecMsg,gIfCtl.AckRecDataBuf,RecLen);
    *pMsgRec=IPC_APP_MEM_OFFSET(pRecMsg);
    *pLenRec=RecLen-sizeof(IPC_HEAD);

    UNLOCK(gSendSynMutex); 
    return IPC_SUCCESS;
    
}

ULONG ipc_if_send_asynmsg(USHORT usDstMo,char *pAppMsgSend,USHORT usLenSend
           ,UCHAR ucMsgType)
{
    IPC_HEAD  *pIpcHead;
    int AddrLen;
    int Ret;
    USHORT usSn;

    if(pAppMsgSend==NULL)
    {
        return IPC_NULL_PTR;
    }
    pIpcHead=(IPC_HEAD  *)IPC_APP_MEM_TO_IPC(pAppMsgSend);
    if(pIpcHead->usMagic!=IPC_HEAD_MAGIC)
    {
        return IPC_INVALID_HEAD;
    }
    if(pIpcHead->NewIpcH==TRUE)
    {
        usSn = ipc_get_seqNo(gIfCtl);
        pIpcHead->usSn=usSn;
        pIpcHead->ucMsgType=ucMsgType;
        pIpcHead->ucSrcMo=gIfCtl.ucSrcMo;
        pIpcHead->ucDstMo=usDstMo;
        pIpcHead->usDataLen = usLenSend;
        pIpcHead->SynFlag=IPC_ASYN_MSG;
        pIpcHead->RCode=IPC_SUCCESS;
    }
    else
    {
        pIpcHead->ucMsgType=ucMsgType;
        pIpcHead->usDataLen = usLenSend;
        pIpcHead->RCode=IPC_SUCCESS;
    }   
    if (sendto(gIfCtl.CmdSockFd, pIpcHead, (sizeof(IPC_HEAD)+usLenSend),
        0, (void*)&gIfCtl.IpcSoAddr, sizeof(struct sockaddr_un)) < 0)
    {
        printf("Send Asyn msg FAIL.\r\n");
        return IPC_SOCKET_SENDTO_FAIL;
    }

    return IPC_SUCCESS;
    
}

ULONG ipc_if_preack(char * pRecMsg, char *pSendMsg)
{
    IPC_HEAD  *pIpcRecHead,*pIpcSendHead;

    if((pRecMsg==NULL)||(pSendMsg==NULL))
    {
        return IPC_NULL_PTR;
    }
    
    pIpcRecHead=(IPC_HEAD  *)IPC_APP_MEM_TO_IPC(pRecMsg);
    if(pIpcRecHead->usMagic!=IPC_HEAD_MAGIC)
    {
        return IPC_INVALID_HEAD;
    }
    
    pIpcSendHead=(IPC_HEAD  *)IPC_APP_MEM_TO_IPC(pSendMsg);
    pIpcSendHead->SynFlag=pIpcRecHead->SynFlag;
    pIpcSendHead->usSn    =pIpcRecHead->usSn;
    pIpcSendHead->ucSrcMo=pIpcRecHead->ucDstMo;
    pIpcSendHead->ucDstMo=pIpcRecHead->ucSrcMo;
    pIpcSendHead->ucMsgType=IPC_MSG_ACK;
    //设置回应不需要再新构造头部信息
    pIpcSendHead->NewIpcH=FALSE;

    return IPC_SUCCESS;
}

void *ipc_if_alloc(ULONG ulLenth)
{
    char *pAlloc;
    IPC_HEAD  *pIpcHead;

    pAlloc=(char*)malloc(IPC_APP_MEM_LEN_TO_ACT_LEN(ulLenth));
    if(pAlloc==NULL)
        return NULL;
    pIpcHead=(IPC_HEAD  *)pAlloc;
    
    pIpcHead->usMagic=IPC_HEAD_MAGIC;
    pIpcHead->Ver = IPC_HEAD_CUR_VER;
    pIpcHead->NewIpcH=TRUE;
    
    return IPC_APP_MEM_OFFSET(pAlloc);
    
}

ULONG ipc_if_free(void* pToFree)
{
    char *pAct;
    IPC_HEAD  *pIpcHead;

    pAct=IPC_APP_MEM_TO_IPC(pToFree);
    pIpcHead = (IPC_HEAD*)pAct;

    if(pIpcHead->usMagic!=IPC_HEAD_MAGIC)
        return IPC_INVALID_HEAD;

    pIpcHead->usMagic=0;
    free(pAct);
    return IPC_SUCCESS;
}

ULONG ipc_if_send_ack(UCHAR ucDstMo,char * PRecMsg,short RetCode,char *pAckData,USHORT AckDataLen)
{
    IPC_APP_MSG * pAckMsg = NULL;
    USHORT AckMsgLen;
    int ret = 0;
        
    //构造回应包
    AckMsgLen = sizeof(IPC_APP_MSG) + AckDataLen;
    pAckMsg = (IPC_APP_MSG *)ipc_if_alloc(AckMsgLen);
    if (pAckMsg == NULL) {
        printf("ipc_if_alloc allocate memory fail!\n");    
        ret = -1;
        goto end;
    }
    if (0 != ipc_if_preack(PRecMsg,(char *)pAckMsg)) {
        printf("ipc_if_preack fail!\n");    
        ret = -1;
        goto end;
    }            
    //消息ID 最高位(APP_MSG_ACK_BIT)为1表示是该消息的回应包
    pAckMsg->MsgHead.MsgID = ((IPC_APP_MSG *)PRecMsg)->MsgHead.MsgID | APP_MSG_ACK_BIT;
    pAckMsg->MsgHead.RetCode = RetCode;
    pAckMsg->MsgHead.DataLen = AckDataLen;
    if (AckDataLen) {
        bcopy(pAckData,pAckMsg->data,AckDataLen);
    }
    //发送
    if (ipc_if_send_asynmsg(ucDstMo,(char *)pAckMsg,AckMsgLen,IPC_MSG_ACK) != 0) {
        printf("IPC send ack fail!\n");    
        ret = -1;
        goto end;
    }
end:        
    if (pAckMsg)
        ipc_if_free(pAckMsg);     
    return ret;
}

UCHAR ipc_if_get_thismoid()
{
    if(gIfCtl.gInitFlag==TRUE)
        return gIfCtl.ucSrcMo;
    return 0;
}


int ipc_if_get_cmd_result(unsigned short dstModuleID,short MsgID,char *rcvbuf,int buflen,char *cmd,int cmdlen,short *pRetCode)
{
    int       ret=0;
    IPC_APP_MSG * pSend;
    USHORT msgLen;
    IPC_APP_MSG * pRcv;
    USHORT rcvLen;

    msgLen = sizeof(IPC_APP_MSG) + cmdlen;
    pSend = (IPC_APP_MSG *)ipc_if_alloc(msgLen);
    if (pSend == NULL)
    {
        printf("error occured!allocate memory fail! MsgID=%d\n",MsgID); 
        return -1;
    }
    
    pSend->MsgHead.MsgID = MsgID;
    pSend->MsgHead.DataLen = cmdlen;
    if (cmd)
        memcpy(pSend->data,cmd,cmdlen);
    
    ret = ipc_if_send_synmsg(dstModuleID,(char *)pSend,msgLen,IPC_MSG_CMD,(char **)&pRcv,&rcvLen);
    ipc_if_free(pSend);
    
    if (ret == IPC_INVALID_MODULE)
    {
        char buf[100];
        sprintf(buf,"error occured!invalid module:%d",dstModuleID);
        printf("%s\n",buf); 
        return -2;
    }
    else if (ret == IPC_SOCKET_SENDTO_FAIL)
    {
        printf("error occured!send message fail! MsgID=%d\n",MsgID); 
        return -3;
    }
    else if (ret == 0)
    {
        if ( (rcvLen - sizeof(IPC_APP_MSG)) != buflen)
        {
            printf("error occured!receive bad message. MsgID=%d, rcvLen=%d expLen=%d\n",MsgID, rcvLen - sizeof(IPC_APP_MSG), buflen); 
            ipc_if_free(pRcv);
            return -5;
        }        
  
        memcpy(rcvbuf,pRcv->data,buflen);
        
        if(pRetCode)
        {
            *pRetCode = pRcv->MsgHead.RetCode;
        }
        
        ipc_if_free(pRcv);
    }
    else
    {
        printf("error occured!ipc return error . MsgID=%d\n",MsgID); 
        return -6;
    }

    return 0;

}

int ipc_if_exe_cmd(unsigned short dstModuleID,short MsgID, char *cmddata,int cmdlen, short *pRetCode)
{
    int       ret=0;
    IPC_APP_MSG * pSend;
    USHORT msgLen;
    IPC_APP_MSG * pRcv;
    USHORT rcvLen;

    msgLen = sizeof(IPC_APP_MSG) + cmdlen;
    pSend = (IPC_APP_MSG *)ipc_if_alloc(msgLen);
    if (pSend == NULL)
    {
        printf("\r\nerror occured!allocate memory fail! MsgID=%d\n",MsgID);
        return -1;
    }
    pSend->MsgHead.MsgID = MsgID;
    pSend->MsgHead.DataLen = cmdlen;
    memcpy(pSend->data,cmddata,cmdlen);
    ret = ipc_if_send_synmsg(dstModuleID,(char *)pSend,msgLen,IPC_MSG_CMD,(char **)&pRcv,&rcvLen);
    ipc_if_free(pSend);
    if (ret == IPC_INVALID_MODULE)
    {
        char buf[100];
        sprintf(buf,"\r\nerror occured!%d not in running!MsgID=%d\n",dstModuleID,MsgID);
        printf("%s",buf); 
        return -2;
    }
    else if (ret == IPC_SOCKET_SENDTO_FAIL)
    {
        printf("\r\nerror occured!send message fail! MsgID=%d\n",MsgID); 
        return -3;
    }
    else if (ret == 0)
    {
        if(pRetCode)
        {
            *pRetCode = pRcv->MsgHead.RetCode;
        }
        ipc_if_free(pRcv);
    }
    else
    {
        printf("\r\nerror occured!ipc return error ! MsgID=%d\n",MsgID); 
        return -5;
    }

    return 0;

}

void * ipc_init_shmem(char *path,int size,int flag,int *shmid)
{
    key_t key;
    void *pshare = NULL;
    if ((key = ftok(path, 1)) < 0) {
        printf("ftok error:%s\n", strerror(errno));
        return NULL;
    }
    if((*shmid = shmget(key, size, flag)) < 0)
    {
        printf("shmget error:%s\n", strerror(errno));
        return NULL;
    }
    if((pshare = (void*)shmat(*shmid, 0, 0)) == (void*)-1)
    {
        printf("shmat error:%s\n", strerror(errno));
        return NULL;
    }
    return pshare;
}

int ipc_del_shmem(void * addr)
{
    return shmdt(addr);
}

int ipc_rm_shmem(int shmid)
{
    return shmctl(shmid,IPC_RMID,0);
}

int ipc_init_sem(char *path,int flag,int *semid)
{
    key_t key;
    if ((key = ftok(path, 1)) < 0) {
        printf("ftok error:%s\n", strerror(errno));
        return -1;
    }
    *semid = semget(key,1,flag);
    if (*semid < 0) {
            printf("couldn't create semaphore\n");
            return -1;
    }
    semctl(*semid,0,SETVAL,1);
    return 0;
}

int ipc_rmv_sem(int semid)
{
    if(semctl(semid,0,IPC_RMID)<0)
    {
        perror("semctl\n");
        return -1;
    }
    return 0;
}

void ipc_sem_down(int semid)
{
    semop(semid,&op_down[0],1);
}
void ipc_sem_up(int semid)
{
    semop(semid,&op_up[0],1);
}

pthread_t ipc_pthread_proc_id()
{
    return proc_thread_id;
}


