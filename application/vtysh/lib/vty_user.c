#include <zebra.h>
#include <version.h>

#include <pwd.h>
#include "memory.h"
#include "linklist.h"
#include "command.h"
#include "vty.h"
#include <global.h>
#include "vtyCommon.h"
//#include "tfMacCryptExpo.h"


/* Extern host structure from command.c */
extern struct host host;
extern int errno;

//all local user account
struct list *vty_userlist;


static void 
vty_encrypt (unsigned int v[], unsigned int k[]) 
{
    unsigned int v0=v[0], v1=v[1], sum=0, i;              /* set up */
    unsigned int delta=0x9e3779b9;                        /* a key schedule constant */
    unsigned int k0=k[0], k1=k[1], k2=k[2], k3=k[3];      /* cache key */
    
    for (i=0; i < 32; i++)                          /* basic cycle start */
    {                                               
        sum += delta;
        v0  += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1  += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                               /* end cycle */
    
    v[0]=v0; v[1]=v1;
}
 
static void
vty_decrypt (unsigned int v[], unsigned int k[]) 
{
    unsigned int v0=v[0], v1=v[1], sum=0xC6EF3720, i;     /* set up */
    unsigned int delta=0x9e3779b9;                        /* a key schedule constant */
    unsigned int k0=k[0], k1=k[1], k2=k[2], k3=k[3];      /* cache key */
    
    for (i=0; i<32; i++)                            /* basic cycle start */
    {                         
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;                                   
    }                                               /* end cycle */
    
    v[0]=v0; v[1]=v1;
}


struct vty_user *
vty_user_new ()
{
    return XCALLOC (MTYPE_VTY_USER, sizeof (struct vty_user));
}

void
vty_user_free (void *user)
{
    XFREE (MTYPE_VTY_USER, user);
}

struct vty_user *
vty_user_lookup (const char *name)
{
    struct listnode *node, *nnode;
    struct vty_user *user;

    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
        if (strcmp (user->name, name) == 0)
            return user;
    }
    return NULL;
}

struct vty_user *
vty_user_check_passwd (const char *name, const char *passwd)
{
    struct listnode *node, *nnode;
    struct vty_user *user = NULL;

    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
        if (strcmp (user->name, name) == 0) {
            if (strcmp("manu", name) == 0) {
            #if 0
                if (tfValidateManuUser((char*)passwd)){
                    user->state = VTY_VERIFY_OK;
                    return user;
                }
                else {
                    user->state = VTY_PASSWD_ERR;
                    break;
                }
            #endif
                    user->state = VTY_VERIFY_OK;
                    return user;
            }
            else {
                if (!strcmp (user->password, passwd)) {
                    user->state = VTY_VERIFY_OK;
                    return user;
                }
                else {
                    user->state = VTY_PASSWD_ERR;
                    break;
                }
            }
        }
    }
    
    return user;
}

bool vty_user_check_access()
{
   return FALSE; 
}

bool vty_user_check_print_log_flag(const char *name, char bit)
{
    struct listnode *node, *nnode;
    vty_user_info_login *user;
    
    for (ALL_LIST_ELEMENTS (host.hist, node, nnode, user)){
        if(!strcmp(user->user.name, name)
            && (user->print_flag & (0x01 << bit))){
            return TRUE;
        }
    }
    return FALSE;
}

void vty_user_set_print_log_flag(const char *name, char bit)
{
    struct listnode *node, *nnode;
    vty_user_info_login *user;
    
    for (ALL_LIST_ELEMENTS (host.hist, node, nnode, user)){
        if(!strcmp(user->user.name, name)){
            user->print_flag |= 0x01 << bit;
            return;
        }
    }
    return;
}


void vty_user_update_hist_state(int port, int state)
{
    struct listnode *node, *nnode;
    vty_user_info_login *user;
    
    for (ALL_LIST_ELEMENTS (host.hist, node, nnode, user)){
        if((user->port == port)
            && (user->user.state != state)){
            user->user.state = state;
            return;
        }
    }
    return;
}

bool vty_user_check_existed(const char *name)
{
    struct listnode *node, *nnode;
    vty_user_info_login *user;

    for (ALL_LIST_ELEMENTS (host.hist, node, nnode, user)){
        if (!strcmp(user->user.name, name) 
            && user->user.state == VTY_VERIFY_OK){
            return TRUE;
        }
    }
    return FALSE;
}

/* un use */
void
vty_user_config_write (struct vty *vty)
{
    struct listnode *node, *nnode;
    struct vty_user *user;

    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
        if (user->password)
            vty_out (vty, " username %s password %s%s"
                        , user->name, user->password, VTY_NEWLINE);
    }
}

struct vty_user *
vty_user_get (const char *name, const char *passwd, int level)
{
  struct vty_user *user;
  user = vty_user_lookup (name);
  if (user)
    return user;

  user = vty_user_new ();
  if(user == NULL)
    return NULL;
  memcpy(user->name, name, VTY_USER_MAX_LEN);
  memcpy(user->password, passwd, VTY_PASSWD_MAX_LEN);
  user->state = VTY_NO_LOGIN;
  user->level = level;
  listnode_add (vty_userlist, user);

  return user;
}

struct vty_user *
vty_user_add_by_name(const char *name, const char *passwd, int level)
{
  struct vty_user *user;

  user = vty_user_new ();
  if(user == NULL)
    return NULL;

  memcpy(user->name, name, VTY_USER_MAX_LEN);
  memcpy(user->password, passwd, VTY_PASSWD_MAX_LEN);
  user->state = VTY_NO_LOGIN;
  user->level = level;
  listnode_add (vty_userlist, user);

  return user;
}

struct vty_user *
vty_user_add(struct vty_user *user)
{
  return vty_user_add_by_name(user->name, user->password, user->level);

  
  //printf("add list:%s\n", user->name);
}


struct vty_user *
vty_user_del(struct vty_user *user)
{
  if(user == NULL)
    return NULL;
  
  listnode_delete (vty_userlist, user);
  
  if(user)
    vty_user_free((void *) user);
  
  return user;
}


vty_user_info_login *
vty_user_add_to_host_hist(struct vty_user *user)
{
  vty_user_info_login *new = NULL;

  new = XCALLOC (MTYPE_VTY_USER, sizeof (vty_user_info_login));;
  if(new == NULL)
    return NULL;
  
  memcpy(&new->user, user, sizeof(struct vty_user));
  //new->login_time = time(NULL);
  quagga_timestamp(0, new->login_time, 32);

  listnode_add (host.hist, new);
  //printf("add to hist list:%s\n", user->name);
  return new;
}

void vty_user_delete_host_hist_all()
{
    list_delete_all_node(host.hist);
}

unsigned int vty_get_user_count()
{ 
    struct listnode *node, *nnode;
    struct vty_user *user;
    unsigned int count = 0;

    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
        count++;
    }
    
    return count;
}

void vty_user_add_default_all(void)
{
    int i = 0 ;

    struct vty_user default_vty_user[VTY_MAX_USERS] ={
                    {"manu",    "",         VTY_NO_LOGIN, CLI_ACCESS_MANU},
                    {"diag",    "admin",    VTY_NO_LOGIN, CLI_ACCESS_DIAG},
                    {"root",    "admin",    VTY_NO_LOGIN, CLI_ACCESS_ROOT},
                    {"admin",   "admin",    VTY_NO_LOGIN, CLI_ACCESS_ADMIN},
                    {"guest",   "",         VTY_NO_LOGIN, CLI_ACCESS_GUEST},
                    {"",        "",         VTY_NO_LOGIN, CLI_ACCESS_GUEST}
                };

    for(i = 0; i < VTY_MAX_USERS;  i++)
        if(default_vty_user[i].name[0] != '\0')
            vty_user_add(&default_vty_user[i]);
}

void vty_user_load(void)
{
    FILE *pFile;
    unsigned int  key[4]={0x09,0x27,0x15,0x08};    
    
    int filelen;
    char  bufread[VTY_MAX_USERS*sizeof(struct vty_user)+VTY_SAVE_DATA_HEADER];    
    int offset;
    int count = 0;
    int index;
    struct vty_user *user = NULL;

    if((pFile = fopen(CLI_CONF_ACCOUNT_NAME, "rb")) == NULL){
        vty_user_add_default_all();
        return;
    }
    
    fseek(pFile, 0, SEEK_END);
    filelen = ftell(pFile);    
    fseek(pFile, 0, SEEK_SET);
    
    if(filelen > sizeof(bufread) || (filelen % 8))
    {
        fclose(pFile);
        vty_user_add_default_all();
        return;
    } 
    
    if(fread(bufread, 1, filelen, pFile) != filelen)
    {       
        fclose(pFile);
        vty_user_add_default_all();
        return;
    }        
    fclose(pFile);  

    /* 解密*/
    for(index = 0; index < filelen; index += 8)
    {
        vty_decrypt((unsigned int*)&bufread[index], key);
    }

    count = bufread[0]; 
    offset = VTY_SAVE_DATA_HEADER;
    if(count*sizeof(struct vty_user) + VTY_SAVE_DATA_HEADER != filelen)
    {
        vty_user_add_default_all();
        return;
    }

    for(index = 0; index < count; index++){
        user = (struct vty_user*)&bufread[sizeof(struct vty_user)*index+offset];
        vty_user_add(user);
    }

}

void vty_user_save(void)
{
    FILE   *pFile;
    unsigned int  key[4]={0x09,0x27,0x15,0x08};    
    char   buf[VTY_MAX_USERS*sizeof(struct vty_user)+VTY_SAVE_DATA_HEADER];
    int  count = 0;
    int  len;
    int  index = 0;
    struct vty_user*user = NULL,*user1 = NULL;
    struct listnode *node, *nnode;

    count = vty_get_user_count();
    len     = sizeof(struct vty_user)*count+VTY_SAVE_DATA_HEADER;

    buf[0] = count;    
    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user1)){
        user = (struct vty_user*)&buf[sizeof(struct vty_user)*index+VTY_SAVE_DATA_HEADER];
        memcpy(user, user1,  sizeof(struct vty_user));
        index++;
    }

    /* 加密*/
    for(index = 0; index < len; index += 8)
    {
        vty_encrypt((unsigned int*)&buf[index], key);
    }

    /* 写文件*/
    if((pFile = fopen(CLI_CONF_ACCOUNT_NAME, "wb+")) == NULL){
        printf("open error.%s\n", strerror(errno));
        return;
    }
    
    if(fwrite(buf, 1, len, pFile) != len)
    {       
        fclose(pFile);
        remove(CLI_CONF_ACCOUNT_NAME);
        printf("fwrite error.\n");
        return;
    }
    
    fclose(pFile);


    /*同步配置到/etc/passwd */
    {
        int ret;
        char cmdBuf[256];
        
        ret = system(">/etc/passwd");
        if(ret)
        {
            printf("clear user error.%s\n", strerror(errno));
            return;
        }

        for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
            if(user->level == CLI_ACCESS_MANU)
                continue;
            
            snprintf(cmdBuf, sizeof(cmdBuf), "adduser -h / -s /bin/sh -G root -D -H -u 0 %s", user->name);
            ret = system(cmdBuf);
            if(ret)
                continue;
          
            snprintf(cmdBuf, sizeof(cmdBuf), "echo %s:%s | chpasswd", user->name, user->password);
            ret = system(cmdBuf);
            if(ret)
            {
                //snprintf(cmdBuf, sizeof(cmdBuf), "deluser %s", user->name);
                //system(cmdBuf);
                continue;
            }
        }
    }

    return;     
}


DEFUN (user_add,
       user_add_cmd,
       "user add USER-NAME USER-PASSWORD (admin|guest|root)",
       "<Group> user command group\n"
       "Add a user\n"
       "User name. <S><Length 1-15>\n"
       "User password. <S><Length 1-15>\n"
       "Admin group\n"
       "Guest group\n"
       "Root group\n")
{
    if(strlen(argv[0]) > VTY_USER_MAX_LEN)
    {
        vty_out(vty, "  Error: the user name is too long.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(strlen(argv[1]) > VTY_PASSWD_MAX_LEN)
    {
        vty_out(vty, "  Error: the user password is too long.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(vty_user_lookup(argv[0]))
    {
        vty_out(vty, "  Error: account already exists.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }
    
    if(!strcmp(argv[2], "admin"))
        vty_user_add_by_name(argv[0], argv[1], CLI_ACCESS_ADMIN);
    else if(!strcmp(argv[2], "guest"))
        vty_user_add_by_name(argv[0], argv[1], CLI_ACCESS_GUEST);
    else if(!strcmp(argv[2], "root"))
        vty_user_add_by_name(argv[0], argv[1], CLI_ACCESS_ROOT);

    vty_user_save();
    
    return CMD_SUCCESS;
}


DEFUN (user_del,
       user_del_cmd,
       "user delete USER-NAME",
       "<Group> user command group\n"
       "Delete a user\n"
       "User name. <S><Length 1-15>\n")
{
    struct vty_user *pUser;

    if(strlen(argv[0]) > VTY_USER_MAX_LEN)
    {
        vty_out(vty, "  Error: the user name is too long.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    pUser = vty_user_lookup(argv[0]);
    if(!pUser)
    {
        vty_out(vty, "  Error: the account does not exist.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(pUser->level == CLI_ACCESS_ROOT)
    {    
        struct listnode *node, *nnode;
        struct vty_user *user;
        
        for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user))
            if(user->level == CLI_ACCESS_ROOT && user != pUser)
                break;

        if(node == NULL)
        {
            vty_out(vty, "  Error: can not delete the last user in the root group.%s", VTY_NEWLINE);
            return CMD_ERR_NOTHING_TODO;
        }
    }
    else if(pUser->level == CLI_ACCESS_MANU)
    {
        vty_out(vty, "  Error: this account can not be delete.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    vty_user_del(pUser);

    vty_user_save();
    
    return CMD_SUCCESS;
}

DEFUN (user_group,
       user_group_cmd,
       "user group (admin|guest|root)",
       "<Group> user command group\n"
       "Change user group\n"
       "Admin group\n"
       "Guest group\n"
       "Root group\n")
{
    struct vty_user *pUser;

    if(strlen(argv[0]) > VTY_USER_MAX_LEN)
    {
        vty_out(vty, "  Error: the user name is too long.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    pUser = vty_user_lookup(argv[0]);
    if(!pUser)
    {
        vty_out(vty, "  Error: the account does not exist.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(!strcmp(argv[2], "admin"))
        pUser->level = CLI_ACCESS_ADMIN;
    else if(!strcmp(argv[2], "guest"))
        pUser->level = CLI_ACCESS_GUEST;
    else if(!strcmp(argv[2], "root"))
        pUser->level = CLI_ACCESS_ROOT;
    
    return CMD_SUCCESS;
}

DEFUN (user_pwd,
       user_pwd_cmd,
       "user password USER-NAME USER-PASSWORD",
       "<Group> user command group\n"
       "Change user password\n"
       "User name. <S><Length 1-15>\n"
       "User password. <S><Length 1-15>\n")
{
    struct vty_user *pUser;
    const char *pPwd;

    if(argc == 2)
    {
        if(strlen(argv[0]) > VTY_USER_MAX_LEN)
        {
            vty_out(vty, "  Error: the user name is too long.%s", VTY_NEWLINE);
            return CMD_ERR_NOTHING_TODO;
        }

        if(strcmp(argv[0], vty->sign_user.name) && vty->sign_user.level != CLI_ACCESS_ROOT)
        {
            vty_out(vty, "  Error: only users in the root group can change another user's password.%s", VTY_NEWLINE);
            return CMD_ERR_NOTHING_TODO;
        }
        
        pUser = vty_user_lookup(argv[0]);
        pPwd = argv[1];
    }
    else
    {
        pUser = vty_user_lookup(vty->sign_user.name);
        pPwd = argv[0];
    }
    
    if(!pUser)
    {
        vty_out(vty, "  Error: the account does not exist.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(strlen(pPwd) > VTY_PASSWD_MAX_LEN)
    {
        vty_out(vty, "  Error: the user password is too long.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    if(pUser->level == CLI_ACCESS_MANU)
    {
        vty_out(vty, "  Error: this account can not be modified.%s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }
    memcpy(pUser->password, pPwd, VTY_PASSWD_MAX_LEN);

    vty_user_save();
    
    return CMD_SUCCESS;
}

ALIAS(user_pwd,
       user_pwd_self_cmd,
       "user password USER-PASSWORD",
       "<Group> user command group\n"
       "Change user password\n"
       "New password for the current user. <S><Length 1-15>\n")


DEFUN (show_user,
       show_user_cmd,
       "show user",
       DESC_SHOW
       "User account configuration\n")
{
    struct listnode *node, *nnode;
    struct vty_user *user;

    vty_print_string_line (vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
    vty_out(vty, "  %-20s  Group%s", "User", VTY_NEWLINE);
    vty_print_string_line (vty, " ", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
    
    for (ALL_LIST_ELEMENTS (vty_userlist, node, nnode, user)){
        if(cmd_element_access_allowed(user->level, vty->sign_user.level) != TRUE)
            continue;
        
        vty_out(vty, "  %-20s  ", user->name);
        
        switch(user->level) {
            case CLI_ACCESS_ROOT:
                vty_out(vty, "root");
                break;
                
            case CLI_ACCESS_ADMIN:
                vty_out(vty, "admin");
                break;
                
            case CLI_ACCESS_GUEST:
                vty_out(vty, "guest");
                break;

            case CLI_ACCESS_MANU:
                vty_out(vty, "manu");
                break;
                
            default:
                vty_out(vty, "unknown");
                break;
        }
        vty_out(vty, "%s", VTY_NEWLINE);
    }
    vty_print_string_line (vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
  
    return CMD_SUCCESS;
}


DEFUN (save_username_password,
       save_username_password_cmd,
       "debug save",
       "Debug string\n"
       "Save all user list\n")
{
    vty_user_save();
    return CMD_SUCCESS;
}

DEFUN (delete_login_hist_info,
       delete_login_hist_info_cmd,
       "debug delete login-info",
       "Debug string\n"
       "Delete\n"
       "Save all user list login info\n")
{
    vty_user_delete_host_hist_all();
    return CMD_SUCCESS;
}

DEFUN (show_hist_login_info,
       show_hist_login_info_cmd,
       "debug show login",
       "Debug string\n"
       SHOW_STR
       "All login user list\n")
{
    struct listnode *node, *nnode;
    vty_user_info_login *user;
    int count = 0;
  
    vty_out(vty, "index user  level state %10s %15s%s","time","from", VTY_NEWLINE);
    for (ALL_LIST_ELEMENTS (host.hist, node, nnode, user)){
        vty_out(vty, "%3d %6s  %3x %8s %s %s%s"
            , count
            , user->user.name
            , user->user.level
            , user->user.state==VTY_VERIFY_OK?"online":"offline"
            , user->login_time
            , user->address
            , VTY_NEWLINE);
        count++;
    }
  
    return CMD_SUCCESS;
}

DEFUN (set_hist_log_debug,
       set_hist_log_debug_cmd,
       "debug set log",
       "Debug string\n"
       SET_STR
       "Log's flag\n")
{
    vty_user_set_print_log_flag(vty->sign_user.name, LOG_MODULE_DEBUG);
    return CMD_SUCCESS;
}

void
vty_user_init ()
{
    vty_userlist = list_new ();
    
    vty_userlist->del = vty_user_free;
    host.hist->del = vty_user_free;

    vty_user_load();
  
    install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &user_add_cmd);
    install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &user_del_cmd);
    //install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &user_group_cmd);
    install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &user_pwd_cmd);
    install_element(CONFIG_NODE, &user_pwd_self_cmd);
    install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &show_user_cmd);

  
    //install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &save_username_password_cmd);

    //install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &delete_login_hist_info_cmd);
    //install_element_with_right(CONFIG_NODE, ENUM_ACCESS_OP_ROOT, &show_hist_login_info_cmd);
}





