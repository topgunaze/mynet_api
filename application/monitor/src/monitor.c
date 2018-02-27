/**************************************************************
 * 文件名称:  monitor.c
 * 作           者:  steven.tian
 * 日           期:  2015.05.15
 * 文件描述:  monitor hardware event
 * 版           本:  V1.00
 * 修改历史:
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/reboot.h>
//#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <linux/kernel.h>

#include "tf_types.h"
#include "monitor.h"
#include "tf_syslog.h"
//#include "tfMacCryptExpo.h"
#include <termios.h>

#define IPC_EVENT
#define PROMPT_ENABLE 1

#ifdef IPC_EVENT
#include "ipc_if.h"
#endif

static unsigned char  gCfgInited  = 0;
static pthread_mutex_t gCfgInitedMutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned char gCfgRestored = FALSE;
static int gRebootSystem = FALSE;


#if PROMPT_ENABLE
static appDesc_t app[] =
{
    /*path       name           argStr          flag                                                                pid     status          exitStatus    eventId           waitflag    prompt*/
    {"/usr/bin", "ipc_ker",     NULL, APP_START_STAGE1 | APP_RESTART_SELF,                                                            0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_IPC_KER,               0, NULL},
    {"/usr/bin", "tfdevctrl",  NULL, APP_START_STAGE2  | /*APP_CLOSE_IO |*/APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT,                     0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_TF,          0, "Start TF Ctrl"},
    {"/usr/bin", "tfdrv",      NULL, APP_START_STAGE2  | /*APP_CLOSE_IO |*/APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT |APP_DISABLE_WTD,    0,  APP_STATUS_INITIAL,    0, 0,           0, "Start TF Drv"},
    {"/usr/bin", "swigmpsn",    NULL, APP_START_STAGE3  | APP_CLOSE_IO | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                          0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_IGMP_SN,            0, "Start Switch App"},
    {"/usr/bin", "swrstp",      NULL, APP_START_STAGE3   | APP_CLOSE_IO | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                          0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_RSTP,               0, NULL},
    {"/usr/bin", "swdhcpsn",    NULL, APP_START_STAGE3 | APP_CLOSE_IO | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_DHCP_SN,            0, NULL},
    {"/usr/bin", "l2",          NULL, APP_START_STAGE3   | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_L2,                 0, NULL},
    {"/usr/bin", "qosacl",      NULL, APP_START_STAGE3  |  APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_QOS_ACL,            0, NULL},
/*   {"/usr/bin", "swdot1x",     NULL, APP_START_STAGE3  | APP_CLOSE_IO | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_DOT1X,              0, NULL}, */
    {"/usr/bin", "swlacp",      NULL, APP_START_STAGE3  | APP_CLOSE_IO | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_LACP,            0, NULL},
    {"/usr/bin", "bcm.user",    NULL, APP_START_STAGE3   |  APP_RESTART_SYSTEM | APP_SHARE_WAIT,                         0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SW_SDK,                0, NULL},
    {"/usr/bin", "sysctrl",     NULL, APP_START_STAGE4  | /*APP_CLOSE_IO |*/ APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT,                     0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_SYSCTRL,               0, "Start System Control"},
    {"/usr/bin", "vtysh",       NULL, APP_START_STAGE4   | APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT,                     0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_CLI,                   0, "Start CLI"},
    {"/bin",     "telnet", "127.0.0.1 23", APP_START_STAGE5 | APP_REDIRECT | APP_RESTART_SELF | APP_TERM_SILENT,                      0,  APP_STATUS_INITIAL,    0, APP_EVENT_ID_TERM,                  0, NULL}
};
#else
static appDesc_t app[] =
{
    /*path       name           argStr          flag                                                                pid     status          exitStatus    eventId           waitflag    prompt*/
    {"/usr/bin", "ipc_ker",     NULL, APP_START_STAGE1 | APP_RESTART_SELF,                                          0,  APP_STATUS_INITIAL,    0, IPC_EVENT_BASE,               0, NULL},
    {"/usr/bin", "tfdevctrl",  NULL, APP_START_STAGE2 | APP_EXCLUSIVE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_TF_INITED,    0, "Start TF Ctrl"},
    {"/usr/bin", "tfdrv",      NULL, APP_START_STAGE2 | APP_EXCLUSIVE_WAIT |APP_DISABLE_WTD,      0,  APP_STATUS_INITIAL,    0, 0,     0, "Start TF Drv"},
    //{"/usr/bin", "swigmpsn",    NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_IGMP_SN_INITED,  0, "Start Switch App"},
    //{"/usr/bin", "swrstp",      NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_RSTP_INITED,     0, NULL},
    //{"/usr/bin", "swdhcpsn",    NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_DHCP_SN_INITED,  0, NULL},
    //{"/usr/bin", "l2",          NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_L2_INITED,       0, NULL},
/*    {"/usr/bin", "swdot1x",     NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_DOT1X_INITED,    0, NULL}, */
    //{"/usr/bin", "swlacp",      NULL, APP_START_STAGE3 | APP_RESTART_SYSTEM | APP_SHARE_WAIT,                       0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SW_LACP_INITED,     0, "Start swlacp2"},
    {"/usr/bin", "bcm.user",    NULL, APP_START_STAGE3 | APP_EXCLUSIVE_WAIT | APP_DISABLE_WTD,                                            0,  APP_STATUS_INITIAL,    0, 0,         0, NULL},
    {"/usr/bin", "sysctrl",     NULL, APP_START_STAGE4 | APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT,                   0,  APP_STATUS_INITIAL,    0, IPC_EVENT_SYSCTRL_INITED,     0, "Start System Control"},
    //{"/usr/bin", "vtysh",       NULL, APP_START_STAGE4 | APP_RESTART_SYSTEM | APP_EXCLUSIVE_WAIT,                   0,  APP_STATUS_INITIAL,    0, IPC_EVENT_CLI_INITED,         0, "Start CLI"},
    //{"/bin",     "telnet", "127.0.0.1 2323", APP_START_STAGE5 | APP_REDIRECT | APP_RESTART_SELF,                    0,  APP_STATUS_INITIAL,    0, IPC_EVENT_BASE,               0, NULL}
};
#endif

static int app_start(appDesc_t *pAppDesc);
static int keep_alive(void);

#ifdef CHECK_FILE_TABLE_ENABLE
/*************************************************************
 * check_file_table - is there file table overflow
 *
 * Returns:
 ************************************************************/
static int check_file_table(void)
{
    int fd = -1;
    int ret = 0;

    do
    {
        /* open a file */
        fd = open("/proc/uptime", O_RDONLY);
        if (fd == -1)
        {
        	int err = errno;

        	if (err == ENFILE)
        	{
        	    /* we need a reboot if ENFILE is returned (file table overflow) */
                #if USE_SYSLOG
        	    syslog(LOG_ERR, "file table overflow detected!\n");
                #endif				/* USE_SYSLOG */

                ret = ENFILE;
        	    break;
        	}
        	else
        	{
                #if USE_SYSLOG
        	    errno = err;
        	    syslog(LOG_ERR, "cannot open /proc/uptime (errno = %d = '%m')", err);
                #else				/* USE_SYSLOG */
        	    perror("monitor");
                #endif				/* USE_SYSLOG */

        	    break;
        	}
        }
        else
        {
        	if (close(fd) < 0)
        	{

                #if USE_SYSLOG
        	    syslog(LOG_ERR, "close /proc/uptime gave errno = %d = '%m'", err);
                #else				/* USE_SYSLOG */
        	    perror("monitor");
                #endif				/* USE_SYSLOG */

        	    break;
        	}
        }
    }while(0);

    return ret;
}
#endif

/*************************************************************
 * tf_cfg_factory - factory function
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int tf_cfg_factory(void)
{
    short       cmdRet = OK;
    int         rc     = OK;

    rc = ipc_if_exe_cmd(MODULE_SYSCTRL,IPC_SYS_CFG_FACTORY,NULL,0,&cmdRet);

    return (rc || cmdRet);
}

/*************************************************************
 * factory_reset_key_monitor - key reset or factory
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int factory_reset_key_monitor(void)
{
	int ret = OK;
#if 0
    int status = soc_drv_key_status();
    int ret = OK;

    if (gRebootSystem)
        return ERROR;

    if (status == KEY_SHORT)
    {
        MONITOR_CRIT("Detect button request to restart!");
        ret = ERROR;
    }
    else if (status == KEY_LONG)
    {
        tf_cfg_factory();
        MONITOR_CRIT("Detect button request to factory!");
        ret = ERROR;
    }

    if (ret == ERROR)
    {
        gRebootSystem = getpid();
    }
#endif
    return ret;
}

/*************************************************************
 * keep_alive - feed watchdog
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int keep_alive(void)
{
    static int wdt = -1;
    int    keepAliveTimeOut = 3;/* keep alive time out 3 second */

    //ipc_if_exe_cmd(MODULE_SWSDK, IPC_SWITCH_DRV_KEEP_ALIVE, NULL, 0, NULL);

    if (wdt < 0)
    {
        wdt = open("/dev/watchdog", O_WRONLY);
        if (!(wdt < 0))
            ioctl(wdt, WDIOC_SETTIMEOUT, &keepAliveTimeOut, sizeof(keepAliveTimeOut));
    }

    if (wdt > 0)
        write(wdt, "\0", 2);

    return 0;//soc_drv_wtd_keep_alive();
}

/*************************************************************
 * do_log_file_save - save log file to flash
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static void do_log_file_save(int force)
{
    time_t time_cur;
    char cmd[128] = {0};
    struct stat statbuf;

    time(&time_cur);
    if(lstat(LOG_SAVED_TMP_FILE, &statbuf) < 0)
    {
        memset((char*)&statbuf, 0, sizeof(statbuf));
    }

    if (force || difftime(time_cur, statbuf.st_mtime) > LOG_SAVED_INTVAL)
    {
        snprintf(cmd, sizeof(cmd), "cd /var/log/ && tar -zcf %s ./messages 2>/dev/null", LOG_SAVED_TMP_FILE);
        system(cmd);
    }
}

/*************************************************************
 * do_check - According to the status determine whether to restart the device
 *
 * status[in]: nonzero need restart system.
 *
 * Returns: none
 ************************************************************/
static void do_check(int status)
{
    if (status != 0)
    {
        /* record log */
        signal(SIGCHLD, SIG_DFL);

        keep_alive();
        /* save log */
        do_log_file_save(TRUE);

        sleep(1);/* wait print notice */


        /* Now stop all processes. */
        kill(-1, SIGTERM);

        /*  */
        keep_alive();

        sleep(5);

        #if 1
        kill(1, SIGTERM);
        #else
        reboot(RB_HALT_SYSTEM);
        #endif

        while(1)
        {
            sleep(1);
        }

        /* wait for hardware watchdog reset */
    }

    keep_alive();
}

/*************************************************************
 * get_app_desc_by_pid - get application descriptor by pid
 *
 * Returns: appDesc_t pointer if find otherwise NULL
 ************************************************************/
static appDesc_t* get_app_desc_by_pid(pid_t pid)
{
    int idx;

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        if ((app[idx].status == APP_STATUS_RUNNING) && (app[idx].pid == pid))
        {
            return (&app[idx]);
        }
    }

    return NULL;
}

static int prompt_print(const char *prompt, char fill, int total, int idx, const char *status)
{
    if ((idx <= 0 && prompt == NULL)
        || total == 0
        || strlen(prompt) >= total)
        return 0;

    if (status)
    {
        printf("\r%*c\r", total, ' ');
        idx = printf("%s ", prompt);
        for (; idx < total; idx++)
            putchar(fill);

        printf(" %s\n", status);
    }
    else
    {
        if (idx > 0 && idx < total)
        {
            putchar(fill);
            idx++;
        }
        else
        {
            if (idx == total)
            {
                printf("\r%*c\r", total, ' ');
            }

            idx = printf("%s %c", prompt, fill);
        }
    }
    fflush(stdout);

    return idx;
}

/*************************************************************
 * set_app_event_flag - get application descriptor by pid
 *
 * Returns: appDesc_t pointer if find otherwise NULL
 ************************************************************/
static void set_app_event_flag(unsigned char eventid)
{
    int idx;

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        if ((app[idx].flag & (APP_EXCLUSIVE_WAIT | APP_SHARE_WAIT)) && (app[idx].eventId == eventid))
        {
            app[idx].waitFlag = TRUE;
            break;
        }
    }

    return;
}

/*************************************************************
 * is_app_wait_timeout - wait application launch timeout
 *
 * Returns: zero: without timeout. non-zero: timeout
 ************************************************************/
static int is_app_wait_timeout(appDesc_t *pAppDesc)
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0)
    {
        return TRUE;
    }
    if (APP_WAIT_TIMEOUT == pAppDesc->timeout)
    {
        pAppDesc->timeout = tp.tv_sec + pAppDesc->timeout;
    }

    return (pAppDesc->timeout < tp.tv_sec);

}

/*************************************************************
 * app_share_wait - get application descriptor by pid
 *
 * Returns: appDesc_t pointer if find otherwise NULL
 ************************************************************/
static int app_share_wait(unsigned int stage)
{
    int idx;
    int rv = FALSE;

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        if ((app[idx].flag & stage)
            && (APP_STATUS_TIMEOUT != app[idx].status)
            && (app[idx].flag & APP_SHARE_WAIT)
            && (app[idx].waitFlag == FALSE))
        {
            if (is_app_wait_timeout(&app[idx]))
            {
                app[idx].status = APP_STATUS_TIMEOUT;
            }
            else
            {
                rv = TRUE;/* contiue to wait */
                break;
            }
        }
    }

    return rv;
}

/*************************************************************
 * app_share_wait - get application descriptor by pid
 *
 * Returns: appDesc_t pointer if find otherwise NULL
 ************************************************************/
static void app_share_wait_print_result(unsigned int stage, char *prompt)
{
    int idx;

    if (NULL == prompt || !IS_VALID_STAGE(stage))
        return;

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        if ((app[idx].flag & stage) && (APP_STATUS_TIMEOUT == app[idx].status))
        {
            prompt_print(prompt, '.', PROMPT_LEN, 0, "FAIL");
            break;
        }
    }

    if (!(idx < sizeof(app) / sizeof(app[0])))
    {
        prompt_print(prompt, '.', PROMPT_LEN, 0, "OK");
    }

    return;
}

/*************************************************************
 * sig_handler - signal handler
 *
 * Returns:
 ************************************************************/
static void sig_handler(int sig)
{
    appDesc_t* pAppDesc = NULL;

    switch(sig)
    {
        case SIGCHLD:
        {
            pid_t   pid;
            int     stat;

            while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
            {
                pAppDesc = get_app_desc_by_pid(pid);
                if (pAppDesc != NULL)
                {
                    pAppDesc->status     = APP_STATUS_EXIT;
                    pAppDesc->exitStatus = stat;/* save child process exit status */
                }
            }
            break;
        }
        case SIGUSR1:
        {

            break;
        }
        default:
        {
            break;
        }
    }
}

/*************************************************************
 * critical_process_deamon - monitor's main function
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int critical_process_deamon(void)
{
    int idx;

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        if (gRebootSystem)
            return -1;

        if (app[idx].status == APP_STATUS_EXIT)
        {
            if (WIFEXITED(app[idx].exitStatus))
            {
                if (!(app[idx].flag & APP_TERM_SILENT))
                {
                    MONITOR_CRIT("%s/%s normal termination, exit code[%d].\n",
                                    app[idx].path, app[idx].name,
                                    WEXITSTATUS(app[idx].exitStatus));
                }

                /* 因为段错误被捕获，导致退出状态不是信号终止 */
                if ((char)WEXITSTATUS(app[idx].exitStatus) == (char)(0 - SIGSEGV))
                {
                    bootFirmwareFailedFlag();
                }
            }
            else if (WIFSIGNALED(app[idx].exitStatus))
            {
                MONITOR_CRIT("%s/%s abnormal termination, signal code[%d].\n",
                                app[idx].path, app[idx].name,
                                WTERMSIG(app[idx].exitStatus));

                /* add by tomson  20150727 */
                bootFirmwareFailedFlag();
            }
            else
            {
                MONITOR_CRIT("%s/%s termination, exit code[%d].\n",
                                app[idx].path, app[idx].name, app[idx].exitStatus);
            }

            if (app[idx].flag & APP_RESTART_SELF)
            {
                app_start(&app[idx]);
            }
            else if (app[idx].flag & APP_RESTART_SYSTEM)
            {
                app[idx].status = APP_STATUS_INITIAL;
                return -1;
            }
        }
        else if (APP_STATUS_TIMEOUT == app[idx].status || APP_STATUS_ERR_START == app[idx].status)
        {
            MONITOR_CRIT("  Start %s/%s failed.\n", app[idx].path, app[idx].name);
            return -1;
        }
    }

    return 0;
}

static int app_arg_build(char *argStr, char **argv, int argc)
{
    char *nextarg = NULL;
    int   idx = 0;

    for (; idx < argc; idx++)
    {
        if ((argv[idx] = strtok_r(argStr, " ", &nextarg)) == NULL)
            break;

        /*fprintf(stderr, "argv[%d] %s %d\n", idx, argv[idx], strlen(argv[idx]));*/

        argStr = NULL;
    }

    return idx;
}

/*************************************************************
 * app_start - start application from app array description
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int app_start(appDesc_t *pAppDesc)
{
    char *argBuf = NULL;
    char *argv[16] = {0};
    pid_t pid = 0;
    char  appName[1024] = {0};
    int fd = -1;
    int ret = 0;
    struct timespec tp;

    #if PROMPT_ENABLE
    //int cnt = 0;
    int prompt_idx = 0;
    //const char prog[] = {'\\','|', '/', '-'};
    #endif

    do
    {
        snprintf(appName, sizeof(appName), "%s/%s", pAppDesc->path, pAppDesc->name);
        argv[0] = (char *)pAppDesc->name;

        /* build arguments vector from string */
        if (pAppDesc->argStr != NULL)
        {
            if ((argBuf = strdup(pAppDesc->argStr)) == NULL)
            {
                ret = -1;
                break;
            }

            app_arg_build(argBuf, &argv[1], sizeof(argv)/sizeof(argv[0]) - 2);

        }
        argv[sizeof(argv)/sizeof(argv[0]) - 1] = NULL;

        /* check wait flag */
        if (pAppDesc->flag & (APP_EXCLUSIVE_WAIT | APP_SHARE_WAIT))
        {
            #ifdef IPC_EVENT
            pAppDesc->waitFlag = FALSE;
            #endif
        }

        /* disable watchdog */
        if (pAppDesc->flag & APP_DISABLE_WTD)
        {
  //          soc_drv_wtd_enable(FALSE);
        }

        /* launch program */
        pid = fork();
        if (pid == 0)/* child */
        {
            if (pAppDesc->flag & APP_REDIRECT)
            {
                fd = open("/dev/tty", O_RDWR);
                if (fd < 0)
                {
                    //printf("can't open /dev/console\n");
                    ret = -1;
                    break;
                }
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            if (pAppDesc->flag & APP_CLOSE_IO)
            {
                fd = open("/dev/null", O_RDWR);
                if (fd < 0)
                {
                    //printf("can't open /dev/console\n");
                    ret = -1;
                    break;
                }
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            execv(appName, argv);
            perror(appName);

            exit(-1);

            break;
        }
        else if (pid > 0)
        {
            /*fprintf(stderr, "start %s successful; PID=%d\n", appName, pid);*/
            pAppDesc->pid    = pid;
            pAppDesc->status = APP_STATUS_RUNNING;
            if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0)
            {
                tp.tv_sec = 0;
            }
            pAppDesc->timeout = tp.tv_sec + APP_WAIT_TIMEOUT;

            #if PROMPT_ENABLE
            if ((pAppDesc->prompt != NULL) && (pAppDesc->prompt[0] != '\0'))
            {
                prompt_idx = prompt_print(pAppDesc->prompt, '.', PROMPT_LEN, 0, NULL);
            }
            #endif

            if (pAppDesc->flag & APP_EXCLUSIVE_WAIT)
            {
                while(pAppDesc->waitFlag == FALSE && !is_app_wait_timeout(pAppDesc))
                {
                    sleep(1);
                    #if PROMPT_ENABLE
                    if (pAppDesc->prompt != NULL)
                    {
                        prompt_idx = prompt_print(pAppDesc->prompt, '.', PROMPT_LEN, prompt_idx, NULL);
                    }
                    #endif
                }

                if (is_app_wait_timeout(pAppDesc))
                {
                    pAppDesc->status = APP_STATUS_TIMEOUT;
                    ret = -1;
                }

                #if PROMPT_ENABLE
                if (pAppDesc->prompt != NULL)
                {
                    prompt_print(pAppDesc->prompt, '.', PROMPT_LEN, prompt_idx, pAppDesc->waitFlag ? "OK" : "FAIL");
                }
                #endif

            }

            /* enable watchdog */
            if (pAppDesc->flag & APP_DISABLE_WTD)
            {
                #if 0
                soc_drv_wtd_enable(TRUE);
                #endif
            }

            if (argBuf != NULL)
            {
                free(argBuf);
                argBuf = NULL;
            }
        }
        else
        {
            pAppDesc->status = APP_STATUS_ERR_START;
            ret = -1;
        }
    }while(0);

    return ret;
}

/*************************************************************
 * app_start - start application from app array description
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
static int apps_start(unsigned int stage)
{
    int idx = 0;
    int fd = -1;
    int ret = 0;
    int shareWait = FALSE;

    #if PROMPT_ENABLE
    //int cnt = 0;
    //const char prog[] = {'\\','|', '/', '-'};
    int sharePromotIdx = sizeof(app);
    int promptIdx = 0;
    #endif

    /* redirect standard input */
    fd = open("/dev/null", O_RDWR);
    if (fd < 0 )
    {
        return -1;
    }
    dup2(fd, STDIN_FILENO);
    close(fd);

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);/* CTRL+C */
    signal(SIGTSTP, SIG_IGN);/* CTRL+Z */
    signal(SIGQUIT, SIG_IGN);/* CTRL+\ */
    signal(SIGCHLD, sig_handler);

    for (idx = 0; idx < sizeof(app) / sizeof(app[0]); idx++)
    {
        /* 指定启动阶段 */
        if (app[idx].flag & stage)
        {
            ret = app_start(&app[idx]);
            //fprintf(stderr, "%s:%d ret=%d\n", __FUNCTION__, __LINE__, ret);
            /* 需要考虑异常处理 */
            if (ret < 0)
            {
                break;
            }

            if (app[idx].flag & APP_SHARE_WAIT)
            {
                shareWait = TRUE;
                if(app[idx].prompt)
                {
                    sharePromotIdx = idx;
                }
            }
        }
    }

    if (shareWait)
    {
        #if PROMPT_ENABLE
        if(sharePromotIdx < sizeof(app) && app[sharePromotIdx].prompt)
        {
            promptIdx = prompt_print(app[sharePromotIdx].prompt, '.', PROMPT_LEN, PROMPT_LEN, NULL);
        }
        #endif

        while(app_share_wait(stage))
        {
            sleep(1);
            #if PROMPT_ENABLE
            if(sharePromotIdx < sizeof(app) && app[sharePromotIdx].prompt)
            {
                promptIdx = prompt_print(app[sharePromotIdx].prompt, '.', PROMPT_LEN, promptIdx, NULL);
            }
            #endif
        }

        #if PROMPT_ENABLE
        if(sharePromotIdx < sizeof(app) && app[sharePromotIdx].prompt)
        {
            app_share_wait_print_result(stage, app[sharePromotIdx].prompt);
        }
        #endif
    }

    /* check program status */
    if (critical_process_deamon() != 0)
    {
        do_check(-1);
    }

    sleep(1);

    return ret;
}

static int setup_sigaction(int signo, void sighandler(int , siginfo_t* , void*))
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = sighandler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(signo, &action, NULL) < 0)
    {
        perror("sigaction error");
        return -1;
    }

    return 0;
}

#if 0
static void signal_usr1_app_wait(int signo, siginfo_t * siginfo, void *context)
{
    appDesc_t* appdesc = NULL;

    appdesc = get_app_desc_by_pid(siginfo->si_pid);

    if (appdesc == NULL)
    {
        return;
    }

    set_app_event_flag(appdesc->eventId);
}
#endif

static void signal_usr1_cfg_wait(int signo, siginfo_t * siginfo, void *context)
{
    appDesc_t* appdesc = NULL;

    appdesc = get_app_desc_by_pid(siginfo->si_pid);

    if (appdesc == NULL)
    {
        return;
    }

    if (strcmp(appdesc->name, "vtysh") == 0)
    {
        gCfgRestored = TRUE;
    }
}

static void signal_usr2_reboot(int signo, siginfo_t * siginfo, void *context)
{
    /*  */
    gRebootSystem = siginfo->si_pid;
}

static void signal_usr1(int signo, siginfo_t * siginfo, void *context)
{
    appDesc_t* appdesc = NULL;
    char apppath[64] = {0};
    FILE * pFile;
    char name[64] = {0};

    #if 0
    fprintf(stderr, "%s: signo=%d\r\n", __func__, signo);

    fprintf(stderr, "si_errno=%d si_code=%d\r\n", siginfo->si_errno, siginfo->si_code);
    fprintf(stderr, "si_pid=%d si_uid=%d\r\n", siginfo->si_pid, siginfo->si_uid);
    #endif
    appdesc = get_app_desc_by_pid(siginfo->si_pid);

    if (appdesc == NULL)
    {
        return;
    }

    set_app_event_flag(appdesc->eventId);
}

static void system_cfg_restore(void)
{
    int pos;
    struct sched_param sched_param;

    setup_sigaction(SIGRTMIN, signal_usr1_cfg_wait);
    //START_CFG_RESTORE();

    /*  */
    #if PROMPT_ENABLE
    pos = prompt_print("Init configuration", '.', PROMPT_LEN, 0, NULL);
    #endif

    /*  */
    sched_param.sched_priority = 90;
    sched_setscheduler(0, SCHED_RR, &sched_param);

    while(1)
    {
        if(!gCfgRestored)
        {
            #if PROMPT_ENABLE
            pos = prompt_print("Init configuration", '.', PROMPT_LEN, pos, NULL);
            #endif
        }
        else
        {
            #if PROMPT_ENABLE
            prompt_print("Init configuration", '.', PROMPT_LEN, pos, "OK");
            #endif
            break;
        }

        do_check(critical_process_deamon());

        sleep(1);
    }

    sched_param.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &sched_param);

    return;
}

static void signal_segv(int signum, siginfo_t* info, void*ptr) {
#if 0
    const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
    char arm_reg[][10]={
        "r0 ($ 0)","r1 ($ 1)","r2 ($ 2)","r3 ($ 3)",
        "r4 ($ 4)","r5 ($ 5)","r6 ($ 6)","r7 ($ 7)",
        "r8 ($ 8)", "r9 ($ 9)","r10($10)", "fp ($11)",
        "ip ($12)", "sp ($13)","lr ($14)", "pc ($15)",
        "cpsr ($16)"
    };

    char buf[256] = {0};
    int  count;
    size_t i;
    ucontext_t *ucontext = (ucontext_t*)ptr;
    unsigned long *arm_r;
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

    fprintf(stderr, "EPC=0x%08lx\n",ucontext->uc_mcontext.arm_pc);
    fprintf(stderr, "RA=0x%08lx\n",ucontext->uc_mcontext.arm_lr);
    arm_r = &ucontext->uc_mcontext.arm_r0;
    for (i = 0; i < 17; i++){
        if (i%2 == 0)
            fprintf(stderr, "\n");
        fprintf(stderr, "%s = 0x%08lx          ", arm_reg[i], *(arm_r++));
    }
    fprintf(stderr,"\n");

#endif
    exit((0-signum));
}


/*************************************************************
 * monitor_init - initialization monitor
 *
 * Returns:
 ************************************************************/
static int monitor_init()
{

    tflog_default = opentflog("MONITOR", 1, TRUE, TFLOG_SYSLOG | TFLOG_STDERR, LOG_DAEMON, NULL);

    /*alarm_event_reg(&ALARM_FAULT_VAL_NAME(tf_sfp));*/

    setup_sigaction(SIGSEGV, signal_segv);

    setup_sigaction(SIGRTMIN, signal_usr1);

    setup_sigaction(SIGUSR2, signal_usr2_reboot);

    return 0;
}

/*************************************************************
 * main - monitor's main function
 *
 * Returns: a negative errno code else positive on success.
 ************************************************************/
int main(int argc, char * argv[])
{
    pid_t pid = 0;
    int detached = 0;
    struct sched_param sched_param;
    struct termios TermIO;

    /* add by tomson  20150727 */
    bootFirmwareSuccessFlag();

    if (argc == 2)
    {
        if (strcasecmp(argv[1], "-d") == 0)
        {
            detached = 1;
        }
    }

    if (detached)
    {
        pid = fork();
    }

    if (!detached || pid == 0)
    {
        /* 关闭串口回显*/
        tcgetattr (STDIN, &TermIO);
        TermIO.c_iflag |= (INLCR | INLCR);
        TermIO.c_lflag &= ~ECHO;
        tcsetattr (STDIN, TCSANOW, &TermIO);

        monitor_init();

        /* stage1: base process */
        apps_start(APP_START_STAGE1);

        /* stage2: tf */
        apps_start(APP_START_STAGE2);

        /* stage3: switch */
        apps_start(APP_START_STAGE3);

        /* stage4: other */
        apps_start(APP_START_STAGE4);

        /* wait configuration restore */
        if (1 == 1)
        {
            system_cfg_restore();

            /* send signal to process group */
            pid = getpid();
            kill(0-pid, SIGCONT);
        }

        /* 打开串口回显*/
        tcflush (STDIN, TCIOFLUSH);
        tcgetattr (STDIN, &TermIO);
        TermIO.c_iflag &= ~(INLCR | INLCR);
        TermIO.c_lflag |= ECHO;
        tcsetattr (STDIN, TCSANOW, &TermIO);

        /* stage5: connect CLI */
        apps_start(APP_START_STAGE5);

        #if 0
        /*  */
        #if PROMPT_ENABLE
        printf("Init configuration .");
        #endif
        #endif

        /* ignore broken pipes */
        signal(SIGPIPE, SIG_IGN);

        /* Set up signal handlers so we can clear up our child processes */
        signal(SIGTERM, SIG_IGN);

        /*  */
        sched_param.sched_priority = 90;
        sched_setscheduler(0, SCHED_RR, &sched_param);

        while(1)
        {
            #ifdef CHECK_FILE_TABLE_ENABLE
            do_check(check_file_table());
            #endif

            /*do_log_file_save(FALSE);*/

            do_check(critical_process_deamon());

            sleep(1);
        }

    }
    else if (detached && pid > 0)
    {
        exit(0);
    }

    return 0;
}

