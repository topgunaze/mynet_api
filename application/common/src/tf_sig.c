/**************************************************************
 * 文件名称:  tf_sig.c
 * 作           者:  steven.tian
 * 日           期:  2016.05.03
 * 文件描述:  system signal encapsulation
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/
#include <stdio.h>
#include <string.h>
#include <signal.h>

/*   I N S T A L L _ S I G N A L _ H A N D L E R   */
/*-------------------------------------------------------------------------
 *
 * signo[in]: signal number
 * flags[in]:
 * hander[in]: signal handler function
 *
 * return: 0 succeed, -1 on error;
-------------------------------------------------------------------------*/
static int install_signal_handler(int signo, int flags, void (*handler)(int)) 
{
    struct sigaction action;

    if (!handler)
        return -1;
    
    sigemptyset(&action.sa_mask);
    action.sa_handler = handler;
    action.sa_flags = flags & (~SA_SIGINFO);
    if (sigaction(signo, &action, NULL) < 0) 
    {
        perror(__func__);
        return -1;
    }

    return 0;
}

/*   I N S T A L L _ S I G N A L _ A C T I O N   */
/*-------------------------------------------------------------------------
 *
 * signo[in]: signal number
 * flags[in]: 
 * hander[in]: signal handler function
 *
 * return: 0 succeed, -1 on error;
-------------------------------------------------------------------------*/
static int install_signal_action(int signo, int flags, void (*handler)(int, siginfo_t *, void *)) 
{
    struct sigaction action;

    if (!handler)
        return -1;
    
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO | flags;
    if (sigaction(signo, &action, NULL) < 0) 
    {
        perror(__func__);
        return -1;
    }

    return 0;
}


/*   C D T _ C F G _ R E S T O R E _ N O T I F I E R   */
/*-------------------------------------------------------------------------
 *
 * callback[in]: call back when configuration restore finish
 *
 * return: 0 succeed, -1 on error;
-------------------------------------------------------------------------*/
int tf_cfg_restore_notifier(void (*callback)(int))
{
    return callback ? install_signal_handler(SIGCONT, 0, callback) : -1;
}


