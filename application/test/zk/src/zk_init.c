/**************************************************************
 * 文件名称:  
 * 作           者:  
 * 日           期:  
 * 文件描述:  
 * 版           本:  
 * 修改历史:
 * <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#if 0
#include "tf_common.h"
#include "sys_debug.h"
#include "tf_log.h"

#include "ipc_if.h"

#include "thread.h"
#include "zebra.h"
#include "command.h"
#include "global.h"
#include "memory.h"
#include "vtysh.h"

#include "tf_monitor_pub.h"
#include "tf_types.h"
#include "gtfCommon.h"
#endif

//#include "gtfDevInit.h"
#include "zk_net.h"

#define GTF_CONFIG_FILENAME    "test.conf"
#define GTF_CONFIG_DIR         "/mnt"

//1. vty命令行 与 配置保存
//2. snmp
//3. 板间通信
//4. 本地通信
//5. log

/**************************************************************
 * 函数名称:
 * 作           者:
 * 日           期:
 *
 * 函数功能:
 * 输入参数:
 * 输出参数:
 * 返回数值:
                            RTN_OK        : 成功
                            RTN_ERROR   : 失败
 * 其           它:
**************************************************************/

int
main(void)
{

        //启动网络任务
#ifdef TF_NET_USE_MP
        if (tf_ctrl_net_mempool_init())
        {
            exit(1);
        }
#endif
    
        tf_ctrl_net_state_init();
        tf_ctrl_net_map_init();
    
        if (tf_ctrl_net_task_init() || tf_ctrl_net_ev_init())
        {
            exit(1);
        }

        while(1)
        {
            sleep(1);
        }

        return 0;

#if 0        

    struct thread_master *master;
    struct thread thread;

    UINT8 module;

    /* GTF 模块IPC 初始化*/
    ipc_if_init();

    /* 向IPC 注册*/
    ipc_if_reg_module(MODULE_GTF, "GTF", (IPC_MSG_CALLBACK)process_gtf_cmd_task);

    /* 关注事件*/
    ipc_if_engage_event(IPC_EVENT_CLI_START);
    ipc_if_engage_event(IPC_EVENT_TF_PORT_UP_DOWN);
    //ipc_if_engage_event(IPC_EVENT_SNMP_AGENT_SWITCH);
    //ipc_if_engage_event(IPC_EVENT_SNMP_TF_PORT_SWITCH);

    /* 1.Prepare master thread. */
    master = thread_master_create ();

    /* 2.Library initialization. */
    tflog_default = opentflog("GTF", GTF_SYSLOG_MODULE_MAX_NUM, TRUE,
                                TFLOG_SYSLOG | TFLOG_CLI, LOG_LOCAL0, alarm_to_vty);

    for(module = 0; module < GTF_SYSLOG_MODULE_MAX_NUM; module++)
        tflog_debug_mod_enable(module, 0);
        
    cmd_init (1);
    vty_init (master);

    /* 3.modules initialization */

    //适配控制板数据库与模块网络接口

    /* 4. Get configuration file. */
    vty_read_config (GTF_CONFIG_FILENAME, GTF_CONFIG_DIR);

    // Create a server, if you want to telnet vtyport
    /* Create VTY's socket .;VTY_TEST_PORT*/
#if 0

    vty_serv_sock (NULL, 0, GTF_VTYSH_PATH);
#else
    //vty_user_init ();
    //vty_serv_sock (NULL, 5678, GTF_VTYSH_PATH);
#endif

    vty_set_config_write(tf_gtf_distributed_config_write);

    /* 发布初始化完成消息*/
    ipc_if_release_event(IPC_EVENT_TF_INITED, NULL, 0);

    TELL_MONITOR("tfdevctrl");

    /* Execute each thread. */
    while (thread_fetch (master, &thread))
        thread_call (&thread); 
     
    /* Not reached. */
    exit(1);
#endif
}


