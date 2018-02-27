/* Virtual terminal [aka TeletYpe] interface routine
   Copyright (C) 1997 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef _ZEBRA_VTY_H
#define _ZEBRA_VTY_H

#include "thread.h"
#include "log.h"
#include "sockunion.h"

#include "sys_common.h"
#include "tf_common.h"
#include "gtfProfileDB.h"
#include "loadExpo.h"
#include "vtyCommon.h"

#define VTY_BUFSIZ 512
#define VTY_MAXHIST 20

enum vty_user_state_t{
    VTY_NO_LOGIN,
    VTY_NO_USER,
    VTY_PASSWD_ERR,
    VTY_VERIFY_OK,
    VTY_LOGOUT,
    VTY_END
};

enum MSG_STA_T
{
    MSG_STA_NONE,
    MSG_STA_DEBUG,
    MSG_STA_ALARM,
    MSG_STA_SEC_END,
    MSG_STA_MAX
};

typedef enum
{
    ACCESS_LEVEL_GUEST,
    ACCESS_LEVEL_MANU   = 1,
    ACCESS_LEVEL_ADMIN  = 2,
    ACCESS_LEVEL_SUPER,
}ENUM_ACCESS_LEVEL;

#define LOG_BUF_USED

/* Start. Add by keith.gong. 2015.08.19*/
#define __RLI_ACCESS_LEVEL_MASK__       0x0000000ff
#define __RLI_ACCESS_OPTION_MASK__      0x0ffffff00
#define __RLI_ACCESS_LEVEL_SHIFT__      8

typedef enum
{
    ENUM_ACCESS_OP_MANUFACTURE  = (1 << __RLI_ACCESS_LEVEL_SHIFT__),
    ENUM_ACCESS_OP_DIAGNOSE     = (ENUM_ACCESS_OP_MANUFACTURE << 1),
    ENUM_ACCESS_OP_ROOT         = (ENUM_ACCESS_OP_DIAGNOSE << 1),
    ENUM_ACCESS_OP_SHELL        = (ENUM_ACCESS_OP_ROOT << 1),

    CLI_ACCESS_MANU   = (ACCESS_LEVEL_MANU  | ENUM_ACCESS_OP_MANUFACTURE ),
    CLI_ACCESS_DIAG   = (ACCESS_LEVEL_ADMIN | ENUM_ACCESS_OP_DIAGNOSE | ENUM_ACCESS_OP_SHELL),
    CLI_ACCESS_ROOT   = (ACCESS_LEVEL_ADMIN | ENUM_ACCESS_OP_ROOT),
    CLI_ACCESS_ADMIN  = (ACCESS_LEVEL_ADMIN),
    CLI_ACCESS_GUEST  = (ACCESS_LEVEL_GUEST),
}CLI_ACCESS_DTE;
/* End. Add by keith.gong. 2015.08.19*/

typedef enum
{
    CLI_STYLE_GENERAL = 1,
    CLI_STYLE_NSCRTV    
}CLI_STYLE_DTE;

/* Vty events */
enum event 
{
    VTY_SERV,
    VTY_READ,
    VTY_WRITE,
    VTY_TIMEOUT_RESET,
#ifdef VTYSH
    VTYSH_SERV,
    VTYSH_READ,
    VTYSH_WRITE,
    VTYSH_CONNECT,
#endif /* VTYSH */
    #ifdef  VTY_EVENT_ALARM
    VTY_ALARM_SERV,
    VTY_ALARM_READ,
    #endif
};

enum
{
    VTY_EXEC_PERMIT = 0,
    VTY_EXEC_DENY = 1,
};

#define VTY_DAEMON_MAX 20

/* VTY shell client structure. */
typedef struct vtysh_client_info_t
{
  int fd;
  char *name;
  int flag;
  char *path;
} vtysh_client_info;


typedef struct model_user_t{
    union
    {
        int test_id;
        short tfId;//和slot_id复用，高2字节slot_id低2字节tfId
        short slot_id;
        short dbaprofile_id;
        short lineprofile_id;
        short srvprofile_id;
        short slaprofile_id;
        short classificationprofile_id;
        short sipagent_profile_id;
        short pots_profile_id;
        short digitmap_profile_id;
        short sipright_profile_id;
        short mvlan_id;
        short vlanif_id;
        int aclId;
    }env;
    union
    {
        DBA_DATA_T  dba;
        LINE_DATA_T lineprofile;
        SRV_DATA_T srvprofile;
        SIPAGENT_DATA_T sipagent_profile;
        POTS_DATA_T pots_profile;
        DIGITMAP_DATA_T digitmap_profile;
        SIPRIGHT_DATA_T sipright_profile;
        IPC_LOAD_FILE_TRANSMIT_T load;
    }data;
}model_user;

#define VTY_USER_MAX_LEN 16
#define VTY_PASSWD_MAX_LEN 16
#define VTY_SAVE_DATA_HEADER 16

#define CLI_ACCOUNT_NAME            "account.tdb"
#define CLI_CONF_ACCOUNT_NAME       "/mnt/cfg/account.tdb"
//#define CLI_CONF_ACCOUNT_NAME       SYSTEM_CONF_PATH CLI_ACCOUNT_NAME
#define CLI_CONF_ACCOUNT_MANU_NAME  "manu"

#define VTYSH_CMD_USER_ENV_GET  "\x00\x00\x00\xA5\xA5" /* memory layout: 0x00 0x00 0x00 0xA5 0xA5 0x00 */

#define VTY_MAX_USERS 10
struct vty_user
{
  char name[VTY_USER_MAX_LEN];
  char password[VTY_PASSWD_MAX_LEN];
  int state;
  int level;
  int style;
  int asign;/*用于保证8个字节对齐*/
};

typedef struct
{
    char ver[VTY_VER_STR_MAX_LEN + 1];
    char user[VTY_USER_MAX_LEN + 1];
}cfg_hdr_t;

typedef struct vty_user_info_login_t{
    struct vty_user user;

    int  print_flag;
    char login_time[32];
    char resv;
    char address[SU_ADDRSTRLEN];
    int  port;
}vty_user_info_login;

enum{
	TERM_TYPE_NONE,
	TERM_TYPE_ANSI,
	TERM_TYPE_linux,
	TERM_TYPE_VT100,
	TERM_TYPE_vt102,
	TERM_TYPE_vt100,
	TERM_TYPE_OTHER,
	TERM_TYPE_END
};

#define MAX_CMD_LINES 8

/* VTY struct. */
struct vty 
{
  /* File descripter of this vty. */
  int fd;

  /* Is this vty connect to file or not */
  /* Add vty shell client , VTY_SHELL_CLIENT, stephen.liu add, 20151008*/
  enum {VTY_TERM, VTY_FILE, VTY_SHELL, VTY_SHELL_SERV, VTY_SHELL_CLIENT, VTY_TERM_LOCAL, VTY_SSH} type;

  /* Node status of this vty */
  int node;

  /* Failure count */
  int fail;

  /* Output buffer. */
  struct buffer *obuf;

  /***Begin: Add by steven.tian on 2015/12/29*************/
  /* Alarm and Debug output buffer. */
  struct buffer *dbuf;
  /***End: Add by steven.tian on 2015/12/29*************/

  /* Command input buffer */
  char *buf;

  /* Command cursor point */
  int cp;

  /* Command length */
  int length;

  /* Command max length. */
  int max;

  /* Histry of command */
  char *hist[VTY_MAXHIST];

  /* History lookup current point */
  int hp;

  /* if previous line, stephen.liu 20161212 */  
  /* temp Command input buffer */
  char *temp_buf;
  int  new_temp_hist;
  /* temp Command cursor point */
  int temp_cp;

  /* temp Command length */
  int temp_length;

  /* temp Command max length. */
  int temp_max;


  /* string control  */
  int string_ctrl;
  /* History insert end point */
  int hindex;

  /* For current referencing point of interface, route-map,
     access-list etc... */
  void *index;

  /* For multiple level index treatment such as key chain and key. */
  void *index_sub;

  /* For escape character. */
  unsigned char escape;

  /* For every vty , will connect to all daemon <vtysh serv>*/
  /* Stephen.Liu 20150930 */
  void *vtysh_info;
  int daemon_num;

  /* For user data, 20151010, stephen.liu add*/
  model_user user;
  struct vty_user sign_user;

  /* Current vty status. */
  enum {VTY_NORMAL, VTY_CLOSE, VTY_MORE, VTY_MORELINE} status;

  /* IAC handling: was the last character received the
     IAC (interpret-as-command) escape character (and therefore the next
     character will be the command code)?  Refer to Telnet RFC 854. */
  unsigned char iac;

  /* IAC SB (option subnegotiation) handling */
  unsigned char iac_sb_in_progress;
  /* At the moment, we care only about the NAWS (window size) negotiation,
     and that requires just a 5-character buffer (RFC 1073):
       <NAWS char> <16-bit width> <16-bit height> */
#define TELNET_NAWS_SB_LEN 8
  unsigned char sb_buf[TELNET_NAWS_SB_LEN];
  /* How many subnegotiation characters have we received?  We just drop
     those that do not fit in the buffer. */
  size_t sb_len;

  int prompt_len;
  /* Window width/height. */
  int width;
  int height;

  /* from command start, control change lines, on ansi terminal type */
  /* stephen.liu 20160120 */
  int cmd_lines;
  /* ANSI and other */
  int terminal_type;
  int tflag;  //0: has not send msg, 1: has send msg

  /* Configure lines. */
  int lines;

  /* Terminal monitor. */
  int monitor;

  /***Begin: Add by steven.tian on 2015/01/25*************/
  /* Terminal debug output. */
  int debug;
  /***End: Add by steven.tian on 2015/01/25*************/

  /* In configure mode. */
  int config;

  /* Read and write thread. */
  struct thread *t_read;
  struct thread *t_write;

  /* Timeout seconds and thread. */
  unsigned long v_timeout;
  struct thread *t_timeout;

  /* What address is this vty comming from. */
  char address[SU_ADDRSTRLEN];
  int  port;

  int  flowctrl;/* limit output speed */
  struct timespec time_limit;

  /* login time */
  struct timeval time_start; /* keith.gong 20151022. record login time*/
};

#define GET_DATA_TIME_OUT 10
#define DEFALUT_BUFFER_SIZE 512

/* #define VTYSH_DEBUG 1 */


/* VTY interactive in command ,stephen.liu 20151028*/
typedef struct vty_interactive_t{
    int flag;

    int vtysh_flag;//in vtysh, all we connect daemons
    int fd;
    int size; //size of buffer
    unsigned char buffer[DEFALUT_BUFFER_SIZE];
}vty_iactive;



/* Integrated configuration file. */
#define INTEGRATE_DEFAULT_CONFIG "fd1616s.cfg"

/* Small macro to determine newline is newline only or linefeed needed. */
/* Stephen.liu add, VTY_SHELL_SERV, and null check, 20151009 */
#define VTY_NEWLINE  \
    ((vty == NULL)||\
    (vty->type == VTY_TERM) ||\
    (vty->type == VTY_TERM_LOCAL) ||\
    (vty->type == VTY_SSH) ||\
    (vty->type== VTY_SHELL_SERV) ? ("\r\n") : ("\n"))
    
//#define VTY_NEWLINE  ("\r\n")

/* Default time out value */
#define VTY_TIMEOUT_DEFAULT 300
//1 stephen.liu 20151010, no timeout//
//#define VTY_TIMEOUT_DEFAULT 0

/* Vty read buffer size. */
#define VTY_READ_BUFSIZ 512

/* Directory separator. */
#ifndef DIRECTORY_SEP
#define DIRECTORY_SEP '/'
#endif /* DIRECTORY_SEP */

#ifndef IS_DIRECTORY_SEP
#define IS_DIRECTORY_SEP(c) ((c) == DIRECTORY_SEP)
#endif

/* GCC have printf type attribute check.  */
#ifdef __GNUC__
#define PRINTF_ATTRIBUTE(a,b) __attribute__ ((__format__ (__printf__, a, b)))
#else
#define PRINTF_ATTRIBUTE(a,b)
#endif /* __GNUC__ */

/* Utility macros to convert VTY argument to unsigned long */
#define VTY_GET_ULONG(NAME,V,STR) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (V) = strtoul ((STR), &endptr, 10); \
  if (*(STR) == '-' || *endptr != '\0' || errno) \
    { \
      vty_out (vty, "%% Invalid "#NAME" value%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    } \
} while (0)

/*
 * The logic below ((TMPL) <= ((MIN) && (TMPL) != (MIN)) is
 * done to circumvent the compiler complaining about
 * comparing unsigned numbers against zero, if MIN is zero.
 * NB: The compiler isn't smart enough to supress the warning
 * if you write (MIN) != 0 && tmpl < (MIN).
 */
#define VTY_GET_INTEGER_RANGE_HEART(NAME,TMPL,STR,MIN,MAX)      \
do {                                                            \
  VTY_GET_ULONG(NAME, (TMPL), STR);                             \
  if ( ((TMPL) <= (MIN) && (TMPL) != (MIN)) || (TMPL) > (MAX) ) \
    {                                                           \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);\
      return CMD_WARNING;                                       \
    }                                                           \
} while (0)

#define VTY_GET_INTEGER_RANGE(NAME,V,STR,MIN,MAX)               \
do {                                                            \
  unsigned long tmpl;                                           \
  VTY_GET_INTEGER_RANGE_HEART(NAME,tmpl,STR,MIN,MAX);           \
  (V) = tmpl;                                                   \
} while (0)

#define VTY_CHECK_INTEGER_RANGE(NAME,STR,MIN,MAX)               \
do {                                                            \
  unsigned long tmpl;                                           \
  VTY_GET_INTEGER_RANGE_HEART(NAME,tmpl,STR,MIN,MAX);           \
} while (0)

#define VTY_GET_INTEGER(NAME,V,STR)                             \
    VTY_GET_INTEGER_RANGE(NAME,V,STR,0U,UINT32_MAX)

#define VTY_GET_IPV4_ADDRESS(NAME,V,STR)                                      \
do {                                                                             \
  int retv;                                                                   \
  retv = inet_aton ((STR), &(V));                                             \
  if (!retv)                                                                  \
    {                                                                         \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);              \
      return CMD_WARNING;                                                     \
    }                                                                         \
} while (0)

#define VTY_GET_IPV4_PREFIX(NAME,V,STR)                                       \
do {                                                                             \
  int retv;                                                                   \
  retv = str2prefix_ipv4 ((STR), &(V));                                       \
  if (retv <= 0)                                                              \
    {                                                                         \
      vty_out (vty, "%% Invalid %s value%s", NAME, VTY_NEWLINE);              \
      return CMD_WARNING;                                                     \
    }                                                                         \
} while (0)

#define VTY_WARN_EXPERIMENTAL()                                               \
do {                                                                          \
  vty_out (vty, "%% WARNING: this command is experimental. Both its name and" \
                " parameters may%s%% change in a future version of Quagga,"   \
                " possibly breaking your configuration!%s",                   \
                VTY_NEWLINE, VTY_NEWLINE);                                    \
} while (0)

/* Exported variables */
extern char integrate_default[];

/* Prototypes. */
extern void vty_init (struct thread_master *);
extern void vty_init_vtysh (void);
extern void vty_terminate (void);
extern void vty_reset (void);
extern struct vty *vty_new (void);
extern int vty_out (struct vty *, const char *, ...) PRINTF_ATTRIBUTE(2, 3);
extern void vty_read_config (char *, char *);
extern void vty_time_print (struct vty *, int);
extern void vty_serv_sock (const char *, unsigned short, const char *);
extern void vty_close (struct vty *);
extern char *vty_get_cwd (void);
extern void vty_log (const char *level, const char *proto, 
                     const char *fmt, struct timestamp_control *, va_list);
extern int vty_config_lock (struct vty *);
extern int vty_config_unlock (struct vty *);
extern int vty_shell (struct vty *);
extern int vty_shell_serv (struct vty *);
extern void vty_hello (struct vty *);

/* Send a fixed-size message to all vty terminal monitors; this should be
   an async-signal-safe function. */
extern void vty_log_fixed (char *buf, size_t len);


extern int  vty_execute (struct vty *vty);
extern char *vty_prompt (struct vty *vty);
extern int _vty_flush (struct vty *vty);
extern void vtysh_show_running_config (struct vty *vty);
int
vtysh_client_daemon_execute (struct vty *vty
        , vtysh_client_info *vclient, const char *line);
int vty_out_to_all(char *str);
void vty_clear_user_data(struct vty *vty);
void vty_user_update_hist_state(int port, int state);

void
vty_clear_buf (struct vty *vty);
void init_vty_all_daemon(struct vty *vty);
int alarm_to_vty(char *str);

int vty_get_data(struct vty *vty, unsigned char *data);
extern void vty_print_string_line (struct vty *vty, char *pString, unsigned int length, char fill);
extern int vty_out_line (struct vty *, const char *, ...) PRINTF_ATTRIBUTE(2, 3);
extern int vtysh_user_env_get_from_serv(struct vty * vty,struct cmd_element *cmd);
extern int vtysh_gtf_profile_node_enter_check(struct vty * vty);

extern char *getNameByPid(pid_t pid, char *task_name) ;
void create_thread_no_arg (void *func);
extern int cfg_header_check(FILE *fp, cfg_hdr_t *hdr);


#endif /* _ZEBRA_VTY_H */
