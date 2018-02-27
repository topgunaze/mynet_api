/* Configuration generator.
   Copyright (C) 2000 Kunihiro Ishiguro

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

#include <zebra.h>

#include "command.h"
#include "linklist.h"
#include "memory.h"

#include "vtysh.h"
#include "tf_log.h"
#include "vtyCommon.h"

vector configvec;

extern int vtysh_writeconfig_integrated;

struct config
{
  /* Configuration node name. */
  char *name;

  /* Configuration string line. */
  struct list *line;

  /* Configuration can be nest. */
  struct config *config;

  /* Index of this config. */
  u_int32_t index;

  /* node 's level , stephen.liu add*/
  int level;
};

typedef struct line_with_level_st
{
    char *line;
    int   level;
}line_with_level_t;

struct list *config_top;
static int config_level = 0;

/*************************************************************
 * ltrim - 删除左边的空格
 * 
 * str[in]: 需要去除空的的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *ltrim(char *str)
{
    if (str == NULL)
        return NULL;

    while(*str != '\0' && *str == ' ')
        str++;

    return str;
}

/*************************************************************
 * rtrim - 删除右边的空格
 * 注意:会修改str内容
 * 
 * str[in]: 需要去除空的的源字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *rtrim(char *str)
{
    char *end = NULL;
    
    if (str == NULL)
        return NULL;

    end = str + strlen(str) - 1;
    while(str != end && *end == ' ')
    {
        *end = '\0';
        end--;
    }

    return str;
}

/*************************************************************
 * lstripNewLine - 删除左边的换行符
 * 
 * str[in]: 需要去除换行符的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *lstripNewLine(char *str)
{
    if (str == NULL)
        return NULL;

    while((*str != '\0') && (*str == '\r' || *str == '\n'))
    {
        str++;
    }

    return str;
}

/*************************************************************
 * rstripNewLine - 删除右边的换行符
 * 注意:会修改str内容
 * 
 * str[in]: 需要去除换行符的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *rstripNewLine(char *str)
{
    char *end = NULL;
    
    if (str == NULL)
        return NULL;

    end = str + strlen(str) - 1;
    while((end != str) && (*end == '\r' || *end == '\n'))
    {
        *end = '\0';
        end--;
    }

    return str;
}

int
line_cmp (char *c1, char *c2)
{
  return strcmp (c1, c2);
}

void
line_del (char *line)
{
  XFREE (MTYPE_VTYSH_CONFIG_LINE, line);
}

void
line_with_level_del (line_with_level_t *line)
{
    XFREE (MTYPE_VTYSH_CONFIG_LINE, line->line);
    XFREE (MTYPE_VTYSH_CONFIG_LINE, line);
}

struct config *
config_new ()
{
  struct config *config;
  config = XCALLOC (MTYPE_VTYSH_CONFIG, sizeof (struct config));
  return config;
}

int
config_cmp (struct config *c1, struct config *c2)
{
  return strcmp (c1->name, c2->name);
}

void
config_del (struct config* config)
{
  list_delete (config->line);
  if (config->name)
    XFREE (MTYPE_VTYSH_CONFIG_LINE, config->name);
  XFREE (MTYPE_VTYSH_CONFIG, config);
}

struct config *
config_get (int index, const char *line)
{
    struct config *config;
    struct config *config_loop;
    struct list *master;
    struct listnode *node, *nnode;

    config = config_loop = NULL;

    master = vector_lookup_ensure (configvec, index);

    if (! master)
    {
        master = list_new ();
        master->del = (void (*) (void *))config_del;
        master->cmp = (int (*)(void *, void *)) config_cmp;
        vector_set_index (configvec, index, master);
    }

    /*
    for (node = master->head, config_loop = NULL; \
        node != NULL && (config_loop = node->data,nnode = node->next, 1); \
        node = nnode, config_loop = NULL)
    */
    for (ALL_LIST_ELEMENTS (master, node, nnode, config_loop))
    {
        if (strcmp (config_loop->name, line) == 0)
            config = config_loop;
    }

    if (! config)
    {
        config = config_new ();
        config->line = list_new ();
        config->line->del = (void (*) (void *))line_with_level_del;
        config->line->cmp = (int (*)(void *, void *)) line_cmp;
        config->name = XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line);
        config->index = index;
        listnode_add (master, config);
    }
    return config;
}

void
config_add_line (struct list *config, const char *line)
{
  listnode_add (config, XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line));
}

void
config_add_line_with_level (struct list *config, const char *line, int level)
{
    line_with_level_t *line_level;
    line_level = XCALLOC (MTYPE_VTYSH_CONFIG_LINE, sizeof (line_with_level_t));
    if (NULL != line_level)
    {
        line_level->line = XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line);
        line_level->level = level;
        listnode_add (config, line_level);
    }
}

void
config_add_line_uniq (struct list *config, const char *line)
{
  struct listnode *node, *nnode;
  char *pnt;

  for (ALL_LIST_ELEMENTS (config, node, nnode, pnt))
    {
      if (strcmp (pnt, line) == 0)
	return;
    }
  listnode_add_sort (config, XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line));
}

void
vtysh_config_parse_line (const char *line)
{
    char c;
    static struct config *config = NULL;
    static u_int32_t prev = 0;
    int vty_node = -1;

    if (! line)
        return;

    c = line[0];

    if (c == '\0')
        return;

    /*fprintf (stderr, "[%s]\n", line); */
    
    switch (c)
    {
        case '!':
        case '#':
            break;
        case ' ':
            /* Store line to current configuration. */
            if (config && config->line)
            {
                //1 Stephen.liu modified 20151009
                config_add_line_with_level (config->line, line, config_level);
            }
            else
                config_add_line_with_level (config_top, line, config_level);
            break;
        default:
            if ((vty_node = vtysh_node_get(line)) >= 0)
            {
                config = config_get(vty_node, line);
                if (config != NULL && (u_int32_t)config != prev) 
                {
                    prev = (u_int32_t)config;
                    config_level++;
                    //add by stephen.liu 20160929
                    config->level = config_level;
                }
            }
            else
            {
                if(prev != (u_int32_t)config_top)
                {
                    prev = (u_int32_t)config_top;
                    config_level++;
                }
                
                config_add_line_with_level (config_top, line, config_level);
                config = NULL;
            }
            break;
    }

}

void
vtysh_config_parse (char *line)
{
    char *begin = NULL;
    char *end = NULL;
    char *pnt = NULL;

    begin = pnt = line;

    while (*pnt != '\0')
    {
        if (*pnt == '\n')
        {
            end = pnt;
            *pnt++ = '\0';
            /* skip terminal character */
            while(begin < pnt && *begin == '\0')
            {
                begin++;
            }
            /* Remove trailing spaces */
            while(begin < end && (*(end - 1) == ' ' || *(end - 1) == '\0'))
            {
                end--;
                *end = '\0';
            }
            if (begin < pnt)
            {
                vtysh_config_parse_line (begin);
            }
            begin = pnt;
        }
        else
        {
            if (*pnt == '\r')
            {
                *pnt = '\0';
            }
            pnt++;
        }
    }
}

/* Macro to check delimiter is needed between each configuration line
 * or not. */
 /*modified by stephen.liu 20151009*/
#define NO_DELIMITER(I) 0 

int vtysh_config_header(struct vty *vty, FILE *fp)
{
    char cmd[512] = {0};
    char tStr[64] = {0};
    int  cmdLen = sizeof(cmd);
    time_t curTime;
    struct tm *ptm = NULL;
    int rv = OK;

    do
    {
        snprintf(cmd, sizeof(cmd), "#Saving user: %.*s", sizeof(vty->sign_user.name), vty->sign_user.name);
        fprintf(fp, "%s\r\n", cmd);
        time(&curTime);
        ptm = localtime(&curTime);
        if (ptm == NULL)
        {
            rv = ERROR;
            break;
        }
        if (strftime(tStr, sizeof(tStr), "%F %T%z", ptm) == 0)
        {
            rv = ERROR;
            break;
        }
        cmdLen = snprintf(cmd, sizeof(cmd), "#Saving time: %s", tStr);
        fprintf(fp, "%s\r\n", cmd);

        /* vtysh version */
        cmdLen = snprintf(cmd, sizeof(cmd), "#Version: %s", VTY_VERSION);
        fprintf(fp, "%s\r\n", cmd);

        #if 0
        /* device type */
        cmdLen = snprintf(cmd, sizeof(cmd), "#Product number: %s", tStr);
        cfgAction(arg, cmd, strlen(cmd));

        /* hardware version */
        cmdLen = snprintf(cmd, sizeof(cmd), "#Hardware version: %s", tStr);
        cfgAction(arg, cmd, strlen(cmd));
        #endif

        fflush(fp);

    }while(0);

    return 0;
}


/* Display configuration to file pointer. */
void
vtysh_config_dump (FILE *fp)
{
    struct listnode *node, *nnode;
    struct listnode *mnode, *mnnode;
    struct config *config;
    struct list *master;
    //char *line;
    unsigned int i;
    char msg[128] = {0};
    line_with_level_t *line_level;
    int level = 0;
    int config_match;
    int line_match;
    int percent = 0;
    int haveCfg = 0;

    for (level = 0; level <= config_level; level++)
    {
        if (config_level != 0)
        {
            if (50 * level / config_level - percent > (10 + rand()%5))
            {
                if(level != config_level)
                {
                percent = 50 * level / config_level;
                snprintf(msg, sizeof(msg), "\r\n The percentage of saved data is: %d%% \r\n", 50 + percent);
                vty_out_to_all(msg);
                }
            }
        }
        for (ALL_LIST_ELEMENTS (config_top, node, nnode, line_level))
        {
            line_match = 0;
            if (line_level->level == level)
            {
                line_match = 1;
                fprintf (fp, "%s\r\n", line_level->line);
                fflush (fp);
            }
        }
        if (line_match)
        {
            fprintf (fp, "!\r\n");
            fflush (fp);
        }

        for (i = 0; i < vector_active (configvec); i++)
            if ((master = vector_slot (configvec, i)) != NULL)
            {
                config_match = 0;
                for (ALL_LIST_ELEMENTS (master, node, nnode, config))
                {
                    line_match = 0;
                    //add by stephen.liu 20160929, for add acl command display
                    //printf("%s %d, configname=%s, configlevel=%d, level=%d\r\n",
                    //        __func__, __LINE__, config->name, config->level, level);
                    #if 0
                    if(!strncmp(config->name, "acl ", strlen("acl ")) && (config->level == level)){
                        line_match = 1;
                        fprintf (fp, "%s\r\n", config->name);
                        fflush (fp);
                    }
                    #endif
                    if(config->level == level)
                    {
                        line_match = 1;
                        fprintf (fp, "%s\r\n", config->name);
                        fflush (fp);
                    }
                    for (ALL_LIST_ELEMENTS (config->line, mnode, mnnode, line_level))
                    {
                        if (line_level->level == level)
                        {
                            if (line_match == 0)
                            {
                                line_match = 1;
                                fprintf (fp, "%s\r\n", config->name);
                                fflush (fp);
                            }
                            fprintf  (fp, "%s\r\n", line_level->line);
                            fflush (fp);
                        }
                    }

                    if (line_match)
                    {
                        config_match = 1;
                        fprintf (fp, " exit\r\n");
                        fflush (fp);
                        if (! NO_DELIMITER (i))
                        {
                            fprintf (fp, "!\r\n");
                            fflush (fp);
                        }
                    }
                }

                if (config_match)
                {
                    haveCfg = 1;
                    if (NO_DELIMITER (i))
                    {
                        fprintf (fp, "!\r\n");
                        fflush (fp);
                    }
                }
            }
    }

    if (haveCfg)
    {
        fprintf (fp, "end\r\n");
        fflush (fp);
    }

    for (i = 0; i < vector_active (configvec); i++)
        if ((master = vector_slot (configvec, i)) != NULL)
        {
            list_delete (master);
            vector_slot (configvec, i) = NULL;
        }
    list_delete_all_node (config_top);
}

void
vtysh_show_running_config (struct vty *vty)
{
    struct listnode *node, *nnode;
    struct listnode *mnode, *mnnode;
    struct config *config;
    struct list *master;
    //char *line;
    line_with_level_t *line_level;
    int level = 0;
    unsigned int i;
    int config_match;
    int line_match;

    for (level = 0; level <= config_level; level++)
    {
        line_match = 0;
        for (ALL_LIST_ELEMENTS (config_top, node, nnode, line_level))
        {
            if (line_level->level == level)
            {
                line_match = 1;
                vty_out(vty, "%s%s", line_level->line, VTY_NEWLINE);
            }
        }

        if (line_match)
        {
            vty_out(vty, "!%s", VTY_NEWLINE);
        }

        for (i = 0; i < vector_active (configvec); i++)
            if ((master = vector_slot (configvec, i)) != NULL)
            {
                config_match = 0;
                for (ALL_LIST_ELEMENTS (master, node, nnode, config))
                {
                    line_match = 0;
                    //add by stephen.liu 20160929, for add acl command display
                    //printf("%s %d, configname=%s, configlevel=%d, level=%d\r\n",
                    //        __func__, __LINE__, config->name, config->level, level);
                    #if 0
                    if(!strncmp(config->name, "acl ", strlen("acl ")) && (config->level == level)){
                        line_match = 1;
                        vty_out(vty, "%s%s", config->name, VTY_NEWLINE);
                    }
                    #endif
                    if (config->level == level)
                    {
                        line_match = 1;
                        vty_out(vty, "%s%s", config->name, VTY_NEWLINE);
                    }
                    //add end
                    for (ALL_LIST_ELEMENTS (config->line, mnode, mnnode, line_level))
                    {
                        if (line_level->level == level)
                        {
                            if (line_match == 0)
                            {
                                line_match = 1;
                                vty_out(vty, "%s%s", config->name, VTY_NEWLINE);
                            }
                            vty_out(vty, "%s%s", line_level->line, VTY_NEWLINE);
                        }
                    }

                    if (line_match)
                    {
                        config_match = 1;
                        vty_out(vty, " exit%s", VTY_NEWLINE);
                        
                        if (! NO_DELIMITER (i))
                        {
                            vty_out(vty, "!%s", VTY_NEWLINE);
                        }
                    }
                }

                if (config_match)
                {
                    if (NO_DELIMITER (i))
                    {
                        vty_out(vty, "!%s", VTY_NEWLINE);
                    }
                }
                
            }
    }

    for (i = 0; i < vector_active (configvec); i++)
        if ((master = vector_slot (configvec, i)) != NULL)
        {
            list_delete (master);
            vector_slot (configvec, i) = NULL;
        }
    
    list_delete_all_node (config_top);
}

/* Read up configuration file from file_name. */
static void
vtysh_read_file (FILE *confp)
{
    int ret;
    struct vty *vty;

    vty = vty_new ();
    vty->fd = dup (STDERR_FILENO);  /* vty_close() will close this */
    if (vty->fd < 0) {
        /* Fine, we couldn't make a new fd. vty_close doesn't close stdout. */
        vty->fd = STDOUT_FILENO;
    }
    vty->type = VTY_FILE;
    vty->node = VIEW_NODE;

    vty->sign_user.level = CLI_ACCESS_ROOT;

    init_vty_all_daemon(vty);

    vtysh_cmd_execute (vty, "enable");
    vtysh_cmd_execute (vty, "config");

#ifdef __FACTORY_VER__
    vtysh_cmd_execute (vty, "dba-profile profile-id 1");
    vtysh_cmd_execute (vty, " type4 max 1000000");
    vtysh_cmd_execute (vty, " commit");
    vtysh_cmd_execute (vty, "exit");
    vtysh_cmd_execute (vty, "ont-lineprofile profile-id 1");
    vtysh_cmd_execute (vty, " llid 1 dba-profile-id 1");
    vtysh_cmd_execute (vty, " commit");
    vtysh_cmd_execute (vty, "exit");
    vtysh_cmd_execute (vty, "ont-srvprofile profile-id 1");
    vtysh_cmd_execute (vty, " ont-port eth 4 pots 0");
    vtysh_cmd_execute (vty, " commit");
    vtysh_cmd_execute (vty, "exit");
    vtysh_cmd_execute (vty, "interface gtf");
    vtysh_cmd_execute (vty, " ont autofind 1 enable");
    vtysh_cmd_execute (vty, " ont autofind 2 enable");    
    vtysh_cmd_execute (vty, " ont autofind 3 enable");
    vtysh_cmd_execute (vty, " ont autofind 4 enable"); 
    vtysh_cmd_execute (vty, " ont autofind 5 enable");
    vtysh_cmd_execute (vty, " ont autofind 6 enable");    
    vtysh_cmd_execute (vty, " ont autofind 7 enable");
    vtysh_cmd_execute (vty, " ont autofind 8 enable"); 
    vtysh_cmd_execute (vty, " ont autofind 9 enable");
    vtysh_cmd_execute (vty, " ont autofind 10 enable");    
    vtysh_cmd_execute (vty, " ont autofind 11 enable");
    vtysh_cmd_execute (vty, " ont autofind 12 enable"); 
    vtysh_cmd_execute (vty, " ont autofind 13 enable");
    vtysh_cmd_execute (vty, " ont autofind 14 enable");    
    vtysh_cmd_execute (vty, " ont autofind 15 enable");
    vtysh_cmd_execute (vty, " ont autofind 16 enable");    
    vtysh_cmd_execute (vty, "exit");    
#endif

    /* Execute configuration file. */
    ret = vtysh_config_from_file (vty, confp);

    vtysh_cmd_execute (vty, "end");

    vty_close (vty);

    if (ret != CMD_SUCCESS) 
    {
        switch (ret)
        {
            case CMD_ERR_AMBIGUOUS:
                fprintf (stderr, "Ambiguous command.\n");
                break;
            case CMD_ERR_NO_MATCH:
                fprintf (stderr, "There is no such command.\n");
                break;
        }
        fprintf (stderr, "Error occured during reading below line.\n%s\n", vty->buf);
    }
}

/* return value: 0 bad file; 1 good; */
int vtysh_config_file_check(char *config_file)
{
    FILE *fp = NULL;
    char lineBuf[VTY_BUFSIZ + 1] = {0};
    long fileSize = 0;
    char *term = NULL;
    int  ret = 0;
    long readLen = 0;
    cfg_hdr_t cfgHdr;

    do
    {
        if (!config_file)
            break;
        
        fp = fopen(config_file, "r");
        if (!fp)
            break;

        ret = cfg_header_check(fp, &cfgHdr);
        if (ret < 0)
        {
            fprintf(stderr, "%s: %s file header is invalid!\r\n", __func__, config_file);
            ret = 0;
            break;
        }

        /* 老版本配置不带end */
        if ('\0' == cfgHdr.ver[0])
        {
            fprintf(stderr, "%s: %s is old version\r\n", __func__, config_file);
            ret = 1;
            break;
        }
        
        fseek(fp, 0, SEEK_END);

        fileSize = ftell(fp);
        readLen = fileSize > VTY_BUFSIZ ? VTY_BUFSIZ : fileSize;

        fseek(fp, fileSize - readLen, SEEK_SET);

        if (fread(lineBuf, readLen, 1, fp) && ferror(fp))
        {
            fprintf(stderr, "Read file failed!\n");
            break;
        }

        /* 去掉末尾换行符 */
        rstripNewLine(lineBuf);

        /* 找到最后一行的起始位置 */
        if((term = strrchr(lineBuf, '\n')) == NULL)
        {
            break;
        }
        term++;

        /* 可能存在\n\r */
        term = lstripNewLine(term);
        term = ltrim(rtrim(term));
        
        if (0 == strcmp(term, "end"))
        {
            ret = 1;
        }
    }while(0);

    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return ret;
}

/* Read up configuration file from config_default_dir. */
int
vtysh_read_config (char *config_default_dir)
{
  FILE *confp = NULL;
  char savefile[PATH_MAX] = {0};

  if (!config_default_dir)
    return -1;

  /* 如果不存在备份文件，老版本升级新版本或者是已经重启检测配置成功过 */
  snprintf(savefile, sizeof(savefile), "%s%s", config_default_dir, CONF_BACKUP_EXT);
  if ((access(config_default_dir, F_OK) == 0))
  {
    if (!vtysh_config_file_check(config_default_dir))
    {
        tflog_alarm_crit("The config file has been damaged! Try to recover from the backup file.\r\n");
        /* 检查备份文件完整性 */
        if (0 == access(savefile, F_OK))
        {
            /* 老版本升级新版本时，备份配置不带end结尾
                          如果此时新版本配置文件损坏，备份配置不会生效
                    */
            if (!vtysh_config_file_check(savefile))
            {
                tflog_alarm_crit("Recover configuration failed from backup file!\r\n");
                return -1;
            }
            unlink(config_default_dir);
            /* 恢复上一次保存的配置 */
            rename(savefile, config_default_dir);

            tflog_alarm_crit("Successfully recover configuration from backup file!\r\n");
        }
    }
    else
    {
        /* 删除备份文件 */
        unlink(savefile);
    }
    
  }
  else
  {
    return -1;
  }
  

#ifdef __FACTORY_VER__
  confp = fopen (config_default_dir, "wr");
  if (confp == NULL)
    return (1);

  fseek(confp, 0, SEEK_SET);
#else
  confp = fopen (config_default_dir, "r");
  if (confp == NULL)
    return (1);
#endif

  vtysh_read_file (confp);
  fclose (confp);

#ifdef __FACTORY_VER__
  remove(config_default_dir);
#endif
  
  host_config_set (config_default_dir);

  return (0);
}

/* We don't write vtysh specific into file from vtysh. vtysh.conf should
 * be edited by hand. So, we handle only "write terminal" case here and
 * integrate vtysh specific conf with conf from daemons.
 */
void
vtysh_config_write ()
{
  char line[81];
  extern struct host host;

  if (host.name)
    {
      sprintf (line, "hostname %s", host.name);
      vtysh_config_parse_line(line);
    }
  if (vtysh_writeconfig_integrated)
    vtysh_config_parse_line ("service integrated-vtysh-config");
}

void vtysh_config_start()
{
    config_level = 0;
}

void
vtysh_config_init ()
{
  config_top = list_new ();
  config_top->del = (void (*) (void *))line_del;
  configvec = vector_init (1);
}
