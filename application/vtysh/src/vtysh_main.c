/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <version.h>
#include "getopt.h"
#include "command.h"
#include "memory.h"

#include "vtysh.h"
#include "vtysh_user.h"
#include <global.h>

#include "tf_log.h"
#include "tf_monitor_pub.h"
#include "tfNvramParam.h"

/* VTY shell program name. */
char *progname;

/* Configuration file name and directory.  */
char config_default[] =SYSCONFDIR VTYSH_DEFAULT_CONFIG;
char history_file[MAXPATHLEN];

/* Flag for indicate executing child command. */
int execute_flag = 0;
static int gStartCfgRestore = 0;

#if VTYSH_ENABLE_CONSOLE
/* For sigsetjmp() & siglongjmp(). */
static sigjmp_buf jmpbuf;

/* Flag for avoid recursive siglongjmp() call. */
static int jmpflag = 0;

/* A static variable for holding the line. */
static char *line_read;
#endif

/* Master of threads. */
struct thread_master *master;

/* Command logging */
FILE *logfile;

#if VTYSH_ENABLE_CONSOLE
/* SIGTSTP handler.  This function care user's ^Z input. */
void
sigtstp (int sig)
{
  /* Execute "end" command. */
  //vtysh_execute ("end");
  
  /* Initialize readline. */
  rl_initialize ();
  printf ("\n");

  /* Check jmpflag for duplicate siglongjmp(). */
  if (! jmpflag)
    return;

  jmpflag = 0;

  /* Back to main command loop. */
  siglongjmp (jmpbuf, 1);
}

/* SIGINT handler.  This function care user's ^Z input.  */
void
sigint (int sig)
{
  /* Check this process is not child process. */
  if (! execute_flag)
    {
      rl_initialize ();
      printf ("\n");
      rl_forced_update_display ();
    }
}

/* Signale wrapper for vtysh. We don't use sigevent because
 * vtysh doesn't use threads. TODO */
RETSIGTYPE *
vtysh_signal_set (int signo, void (*func)(int))
{
  int ret;
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = func;
  sigemptyset (&sig.sa_mask);
  sig.sa_flags = 0;
#ifdef SA_RESTART
  sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */

  ret = sigaction (signo, &sig, &osig);

  if (ret < 0) 
    return (SIG_ERR);
  else
    return (osig.sa_handler);
}

/* Initialization of signal handles. */
void
vtysh_signal_init ()
{
  vtysh_signal_set (SIGINT, sigint);
  vtysh_signal_set (SIGTSTP, sigtstp);
  vtysh_signal_set (SIGPIPE, SIG_IGN);
}
#endif

#if 0
/* Help information display. */
static void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    printf ("Usage : %s [OPTION...]\n\n" \
	    "Integrated shell for Quagga routing software suite. \n\n" \
	    "-b, --boot               Execute boot startup configuration\n" \
	    "-c, --command            Execute argument as command\n" \
	    "-d, --daemon             Connect only to the specified daemon\n" \
	    "-E, --echo               Echo prompt and command in -c mode\n" \
	    "-C, --dryrun             Check configuration for validity and exit\n" \
	    "-h, --help               Display this help and exit\n\n" \
	    "Note that multiple commands may be executed from the command\n" \
	    "line by passing multiple -c args, or by embedding linefeed\n" \
	    "characters in one or more of the commands.\n\n" \
	    "Report bugs to %s\n", progname, ZEBRA_BUG_ADDRESS);

  exit (status);
}
#endif

/* VTY shell options, we use GNU getopt library. */
struct option longopts[] = 
{
  { "boot",                 no_argument,             NULL, 'b'},
  /* For compatibility with older zebra/quagga versions */
  { "eval",                 required_argument,       NULL, 'e'},
  { "command",              required_argument,       NULL, 'c'},
  { "daemon",               required_argument,       NULL, 'd'},
  { "echo",                 no_argument,             NULL, 'E'},
  { "dryrun",		    no_argument,	     NULL, 'C'},
  { "help",                 no_argument,             NULL, 'h'},
  { "noerror",		    no_argument,	     NULL, 'n'},
  { 0 }
};


#if 0
static void log_it(const char *line)
{
  time_t t = time(NULL);
  struct tm *tmp = localtime(&t);
  char *user = getenv("USER") ? : "boot";
  char tod[64];

  strftime(tod, sizeof tod, "%Y%m%d-%H:%M.%S", tmp);
  
  fprintf(logfile, "%s:%s %s\n", tod, user, line);
}
#endif

/* Pacify zclient.o in libzebra, which expects this variable. */
struct thread_master *master;

extern void cli_alarm_log_print(void *arg);
extern void thread_process_alarm_recv(void);
extern int vty_serv_un_alarm (const char *path);

void thread_process_vty()
{
    struct thread thread;
    
    /* Execute each thread. */
    while (thread_fetch (master, &thread))
      thread_call (&thread);

}


#if VTYSH_ENABLE_CONSOLE
static void
cb_linehandler (char *line)
{
    HIST_ENTRY *last;
    /* If the buffer has already been allocated, return the memory
    * to the free pool. */
    if (line_read){
        free (line_read);
        line_read = NULL;
    }
    
    /* If the line has any text in it, save it on the history. But only if
    * last command in history isn't the same one. */
    if (line && *line){
        using_history();
        last = previous_history();
        if (!last || strcmp (last->line, line) != 0) {
	        add_history (line);
	        append_history(1,history_file);
        }
    }
    
    vtysh_set_timeout_flag(FALSE);
    if(vtysh->node == AUTH_NODE){
        printf("\n");
        fflush(stdout);
    }
    vtysh_execute (line);
    rl_set_prompt((char *)vty_prompt (vtysh));
    
    free (line);
}


#  define USEC_PER_SEC 1000000

int vtysh_rl_select(void)
{
    fd_set fds;
    int r;
    int running = 1;
    struct timeval timeout,ti;

    timeout.tv_sec = vtysh->v_timeout; 
    timeout.tv_usec = 0; 
    /* Install the line handler. */
    rl_callback_handler_install ((char *)vty_prompt (vtysh), cb_linehandler);

    /* Enter a simple event loop.  This waits until something is available
     to read on readline's input stream (defaults to standard input) and
     calls the builtin character read callback to read it.  It does not
     have to modify the user's terminal settings. */
    running = 1;
    while (running){
        FD_ZERO (&fds);
        FD_SET (fileno (rl_instream), &fds);    
        ti = timeout;
        r = select (FD_SETSIZE, &fds, NULL, NULL, &ti);
        if (r < 0){
            if(EINTR == errno){
                continue;
            }
            perror ("rltest: select");
            rl_callback_handler_remove ();
            break;
        }
        if(r == 0){
            //printf("time out vty.\n");
            //timeout
            vtysh_timeout(0);
            continue;
        }
    
        if (FD_ISSET (fileno (rl_instream), &fds)){
            rl_callback_read_char ();
        }
    }
    
    printf ("rltest: Event loop has exited\n");
    return 0;
}

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *
vtysh_rl_gets ()
{
    HIST_ENTRY *last;
    /* If the buffer has already been allocated, return the memory
    * to the free pool. */
    if (line_read){
        free (line_read);
        line_read = NULL;
    }
     
    /* Get a line from the user.  Change prompt according to node.  XXX. */
    line_read = readline ((char *)vty_prompt (vtysh));
     
    /* If the line has any text in it, save it on the history. But only if
    * last command in history isn't the same one. */
    if (line_read && *line_read){
        using_history();
        last = previous_history();
        if (!last || strcmp (last->line, line_read) != 0) {
	        add_history (line_read);
	        append_history(1,history_file);
        }
    }
     
    return (line_read);
}
#endif

extern bool g_vty_init_alarm;

#ifdef SIG_HANDLER
static void sig_handler(int sig)
{
    fprintf(stderr, "PID[%d] signo=%d\r\n", getpid(), sig);
}

char arm_reg[][10]={
    "r0 ($ 0)","r1 ($ 1)","r2 ($ 2)","r3 ($ 3)",
    "r4 ($ 4)","r5 ($ 5)","r6 ($ 6)","r7 ($ 7)",
    "r8 ($ 8)", "r9 ($ 9)","r10($10)", "fp ($11)",
    "ip ($12)", "sp ($13)","lr ($14)", "pc ($15)",
    "cpsr ($16)","ORIG_r0($17)"
};


static void signal_segv(int signum, siginfo_t* info, void*ptr) {
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
    char buf[256] = {0};
    int  count;
    size_t i;
    ucontext_t *ucontext = (ucontext_t*)ptr;
    unsigned long *arm_r;
    /*¨°¨°?a¡Á¨®¡ä¨²¨¤¨ª?¨²o¨®¨¬¡§???¡¥?¨¢1?¡À?stdout/stdin/stderr,?a¨¤???D?¡ä¨°?a*/
    stderr = fopen("/dev/console","w");
    count = readlink( "/proc/self/exe", buf, 256 );
    buf[ count ] = '\0';
    fprintf(stderr, "path:%s\r\n",buf);
    fprintf(stderr, "\nSegmentation Fault!\r\n");
    fprintf(stderr, "info.si_signo = %d\r\n", signum);
    fprintf(stderr, "info.si_errno = %d\r\n", info->si_errno);
    fprintf(stderr, "info.si_code = %d (%s)\r\n", info->si_code, si_codes[info->si_code]);
    fprintf(stderr, "info.si_addr = %p\r\n", info->si_addr);

    fprintf(stderr, "EPC=0x%08lx\r\n",ucontext->uc_mcontext.arm_pc);
    fprintf(stderr, "RA=0x%08lx\r\n",ucontext->uc_mcontext.arm_lr);
    arm_r = &ucontext->uc_mcontext;
    for (i = 0; i < NGREG; i++){
          if (i%2 == 0)
          fprintf(stderr, "\r\n");
          fprintf(stderr, "%s = 0x%08lx          ", arm_reg[i], *(arm_r++));
    }
    fprintf(stderr,"\r\n");
    
    exit((0-signum));
}

int setup_sigsegv(void) {
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
#endif

static void signal_usr1(int signo, siginfo_t * siginfo, void *context) 
{
    gStartCfgRestore = 1;
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

static int parent_is_monitor(void)
{
    pid_t ppid = getppid();
    char  buf[128] = {0};
    FILE* fp = NULL;
    int   ret = 0;

    do {
        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", ppid);
        fp = fopen(buf, "r");
        memset(buf, 0, sizeof(buf));
        if (fp != NULL)
        {
            if (fgets(buf, sizeof(buf), fp) != NULL && strstr(buf, "monitor"))
            {
                ret = 1;
            }
            fclose(fp);
        }
    }while(0);

    return ret;
}

void cli_client_init(void)
{
    /* 1.Prepare master thread. */
    master = (struct thread_master *)thread_master_create ();
    /* 2.Library initialization. */
    //zprivs_init (&ripd_privs);
    //signal_init (master, array_size(ripd_signals), ripd_signals);
    cmd_init (0);
    vty_init (master);
    vty_user_init();
    vty_user_save();
    tf_vty_snmp_init();

    tflog_default = opentflog("VTYSH", 1, TRUE, TFLOG_SYSLOG | TFLOG_CLI, LOG_LOCAL3, alarm_to_vty);
    
    vtysh_config_init ();
    memory_init ();
    vtysh_init();
    vtysh_init_cmd();

    /* 3.modules initialization */
    //test_zclient_init();
    
    /* 4. Get configuration file. */
    vty_read_config (NULL, config_default);  
    /* 5. Change to the daemon program. */
    #if 0
    if (daemon_mode && daemon (0, 0) < 0)
      {
        zlog_err("RIPd daemon failed: %s", strerror(errno));
        exit (1);
      }
    #endif

    // Create a server, if you want to telnet vtyport
    /* Create VTY's socket */
    //printf("sock start.\n");
    vty_serv_sock (NULL, VTYSH_PORT, ALARM_VTYSH_PATH);
    vty_serv_sock (NULL, 0, VTYSH_VTYSH_PATH);
    
    // printf("sock over.\n");
    #if 0
    vty_serv_un_alarm(ALARM_VTYSH_PATH);
    #else
    create_thread_no_arg(thread_process_alarm_recv);
    #ifdef LOG_BUF_USED
    create_thread_no_arg(cli_alarm_log_print);
    #endif
    #endif

    if (parent_is_monitor())
        setup_sigaction(SIGRTMIN, signal_usr1);

    TELL_MONITOR("vtysh");

    /**/
    if (parent_is_monitor())
    {
        int timeout = 10;/* 10s */
        while(!gStartCfgRestore && timeout-- > 0)
            sleep(1);
    }
    /* 6. Get configuration file. */
    vtysh_read_config (integrate_default); 

    /* configuration finished */
    TELL_MONITOR("vtysh");
    #ifdef VTYSH_DEBUG
    extern int g_vtysh_init_complete;
    g_vtysh_init_complete = TRUE;
    #endif
#if VTYSH_ENABLE_CONSOLE
    create_thread_no_arg(thread_process_vty);
#else
    thread_process_vty();
#endif

    while(!g_vty_init_alarm);
    /* Not reached. */
    return;
}

void
cli_ssh_server_init()
{
    system("/sbin/ifconfig lo 127.0.0.1 netmask 255.255.255.255 2>/dev/null");
    system("/sbin/ifconfig lo:1 127.0.0.2 netmask 255.255.255.255 2>/dev/null");
    
    system("mkdir -p /mnt/dropbear 2>/dev/null");
    
    if(access("/mnt/dropbear/dropbear_rsa_host_key", F_OK))
        system("/usr/bin/dropbearkey -t rsa -f /mnt/dropbear/dropbear_rsa_host_key &> /dev/null");
    
    if(access("/mnt/dropbear/dropbear_dss_host_key", F_OK))
        system("/usr/bin/dropbearkey -t dss -f /mnt/dropbear/dropbear_dss_host_key &> /dev/null");

    system("killall ssh 2>/dev/null; ssh");
}

/* VTY shell main routine. */
int
main (int argc, char **argv, char **env)
{
    //tfNvramParamInit();
    cli_ssh_server_init();
    
    cli_client_init();

#if VTYSH_ENABLE_CONSOLE
    /* Signal and others. */
    vtysh_signal_init ();

    //printf("debug:%d\n", __LINE__);
    //start vtysh
    vtysh_init_vty();
    //printf("debug:%d\n", __LINE__);
    vty_hello (vtysh);
    //printf("debug:%d\n", __LINE__);

    vtysh_readline_init();

    snprintf(history_file, sizeof(history_file), "%s/.history_quagga", getenv("HOME"));
    read_history(history_file);
    //printf("debug:%d\n", __LINE__);
    /* Main command loop. */
    //while (vtysh_rl_gets())
    //   vtysh_execute (line_read);

    vtysh_rl_select ();
    
    history_truncate_file(history_file,1000);
    printf ("\n");
#endif

    return 0;
}

#define END


