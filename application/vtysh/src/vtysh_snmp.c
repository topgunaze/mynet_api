/******************************************************************************

  Copyright (C), 2016-2026, C-DATA Co., Ltd.

 ******************************************************************************
  File Name     : vtysh_snmp.c
  Version       : Initial Draft
  Author        : zh.weng
  Created       : 2016/4/6
  Last Modified :
  Description   : vtysh_snmp source file
  History       :
  1.Date        : 2016/4/6
    Author      : zh.weng
    Modification: Created file

******************************************************************************/


#include "tf_types.h"
#include "zebra.h"
#include "thread.h"
#include "command.h"
#include "global.h"
#include "memory.h"
#include "vtyCommon.h"
#include "sys_common.h"
#include "ipc_if.h"
#include "ipc_public.h"
#include "tf_snmp.h"

SNMP_CONF_INFO_STRU snmpConfInfo;
int reconfig=0;

extern void 
vtysh_config_parse_line (const char *line);


static void
_tf_snmp_reconfig(void)
{
    if(snmpConfInfo.sysInfo.enable)
        system("killall "SNMP_PROGRAM_NAME" 2>/dev/null"";"""SNMP_PROGRAM_NAME);
}


static void
_tf_snmp_stop(
                struct vty *vty)
{
    if(!snmpConfInfo.sysInfo.enable)
    {
        vty_out_line(vty, "  Error: Make configuration repeatedly");
        return;
    }

    snmpConfInfo.sysInfo.enable = 0;
    system("killall "SNMP_PROGRAM_NAME" 2>/dev/null"); 
    ipc_if_release_event(IPC_EVENT_SNMP_AGENT_SWITCH, &snmpConfInfo.sysInfo.enable, 1);
}


static void
_tf_snmp_start(
                struct vty *vty)
{
    if(snmpConfInfo.sysInfo.enable)
    {
        vty_out_line(vty, "  Error: Make configuration repeatedly");
        return;
    }
    
    snmpConfInfo.sysInfo.enable = 1;
    _tf_snmp_reconfig();
    ipc_if_release_event(IPC_EVENT_SNMP_AGENT_SWITCH, &snmpConfInfo.sysInfo.enable, 1);
}


int
tf_snmp_conf_update(
                const char *pFullName,
                UINT8 reconfigFlag)
{
    FILE *pF;
    char buf[512];
    int len;
    int idx;
    char right_char;

    pF = fopen(pFullName, "w");
    if(pF == NULL)
    {
        perror("fopen");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%s", "agentaddress udp:161 \n");
    fwrite(buf, 1, len, pF);

    if(snmpConfInfo.sysInfo.sysName[0])
        len = snprintf(buf, sizeof(buf), "sysname %s \n", snmpConfInfo.sysInfo.sysName);
    else
        len = snprintf(buf, sizeof(buf), "sysname %s \n", SNMP_DEF_SYS_NAME);
    fwrite(buf, 1, len, pF);

    if(snmpConfInfo.sysInfo.sysDesc[0])
        len = snprintf(buf, sizeof(buf), "sysdescr %s \n", snmpConfInfo.sysInfo.sysDesc);
    else
        len = snprintf(buf, sizeof(buf), "sysdescr %s \n", SNMP_DEF_SYS_DESC);
    fwrite(buf, 1, len, pF);

    if(snmpConfInfo.sysInfo.sysContact[0])
        len = snprintf(buf, sizeof(buf), "syscontact %s \n", snmpConfInfo.sysInfo.sysContact);
    else
        len = snprintf(buf, sizeof(buf), "syscontact %s \n", SNMP_DEF_CONTACT);
    fwrite(buf, 1, len, pF);

    if(snmpConfInfo.sysInfo.sysLocation[0])
        len = snprintf(buf, sizeof(buf), "syslocation %s \n", snmpConfInfo.sysInfo.sysLocation);
    else
        len = snprintf(buf, sizeof(buf), "syslocation %s \n", SNMP_DEF_LOCATION);
    fwrite(buf, 1, len, pF);

    len = snprintf(buf, sizeof(buf), "%s", "master agentx \n");
    fwrite(buf, 1, len, pF);

    for(idx = 0; idx < MAX_COMMUNITY; idx++)
    {
        switch(snmpConfInfo.snmpCommunity[idx].type)
        {
            case SNMP_RIGHT_RW:
                right_char = 'w';
                break;
            case SNMP_RIGHT_RO:
                right_char = 'o';
                break;
            default:
                continue;
        }
        len = snprintf(buf, sizeof(buf), "r%ccommunity %s default \n",
                        right_char, snmpConfInfo.snmpCommunity[idx].community);
        fwrite(buf, 1, len, pF);
    }

    for(idx = 0; idx < MAX_TRAP; idx++)
    {
        if(!snmpConfInfo.trapInfo[idx].hostName[0])
            continue;

        len = snprintf(buf, sizeof(buf), "trap2sink %d.%d.%d.%d:%d %s\n",
                                (snmpConfInfo.trapInfo[idx].ipAddress>>24)&0xFF,
                                (snmpConfInfo.trapInfo[idx].ipAddress>>16)&0xFF,
                                (snmpConfInfo.trapInfo[idx].ipAddress>>8)&0xFF,
                                (snmpConfInfo.trapInfo[idx].ipAddress)&0xFF,
                                snmpConfInfo.trapInfo[idx].port,
                                snmpConfInfo.trapInfo[idx].community);
        fwrite(buf, 1, len, pF);
    }

    for(idx = 0; idx < MAX_USM_USER; idx++)
    {
        if(snmpConfInfo.usmUserInfo[idx].name[0])
        {
            len = snprintf(buf, sizeof(buf), "createUser %s",
                            snmpConfInfo.usmUserInfo[idx].name);

            if(snmpConfInfo.usmUserInfo[idx].authpass[0])
            {
                len += snprintf(buf + len, sizeof(buf), " MD5 \"%s\"",
                            snmpConfInfo.usmUserInfo[idx].authpass);

                if(snmpConfInfo.usmUserInfo[idx].privpass[0])
                {
                    len += snprintf(buf + len, sizeof(buf), " DES \"%s\"",
                                snmpConfInfo.usmUserInfo[idx].privpass);
                }
            }

            len += snprintf(buf + len, sizeof(buf), "\n");
            fwrite(buf, 1, len, pF);

            len = snprintf(buf, sizeof(buf), "group %s usm %s\n",
                    snmpConfInfo.usmUserInfo[idx].group,
                    snmpConfInfo.usmUserInfo[idx].name);
            fwrite(buf, 1, len, pF);
        }
    }

    len = snprintf(buf, sizeof(buf), "%s", "view all included .1 \n");
    fwrite(buf, 1, len, pF);

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(snmpConfInfo.access[idx].group[0])
        {
            len = snprintf(buf, sizeof(buf), "access %s \"\" usm %s exact %s %s %s \n",
                                        snmpConfInfo.access[idx].group,
                                        snmpConfInfo.access[idx].secLevel,
                                        snmpConfInfo.access[idx].readView,
                                        snmpConfInfo.access[idx].writeView,
                                        snmpConfInfo.access[idx].notifyView);
            fwrite(buf, 1, len, pF);
        }
    }

    fclose(pF);

    if(reconfigFlag)
        _tf_snmp_reconfig();

    return 0;
}

static int 
_tf_snmp_trap_set(
                struct vty *vty,
                const char *pHostName,
                const char *pIpAddress,
                const char *pTrapPort,
                const char *pCommunity)
{
    int idx1, idx2, emptyIdx;

    if(strlen(pHostName) > SNMP_TRAP_HOST_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The trap host name length is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(strlen(pCommunity) > SNMP_COMMUNITY_LEN)
    {
        vty_out_line(vty, "  Error: The community length is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    /*hostName�Ѵ���ʱ�����ִ���*/
    for(idx1 = 0; idx1 < MAX_TRAP; idx1++)
    {
        if(!snmpConfInfo.trapInfo[idx1].hostName[0])
            continue;

        if(strncmp(pHostName, snmpConfInfo.trapInfo[idx1].hostName, SNMP_TRAP_HOST_NAME_LEN) == 0)
        {
            for(idx2 = 0; idx2 < MAX_TRAP; idx2++)
            {
                if(!snmpConfInfo.trapInfo[idx2].hostName[0])
                    continue;

                /*(1) ip ��port �ѱ���ͬ��hostNameʹ�ã�������ʾ��Ϣ*/
                if((idx2!=idx1) &&
                     (snmpConfInfo.trapInfo[idx2].ipAddress == ntohl(inet_addr(pIpAddress))) 
                      && (snmpConfInfo.trapInfo[idx2].port == atoi(pTrapPort)))
                {
                    vty_out_line(vty, "  Error: The trap IP and port has been used by other host name!");
                    return CMD_ERR_NOTHING_TODO;
                }
            }

            /*(2) ��Ϣһ�£�ֱ�ӷ��سɹ�*/
             if((snmpConfInfo.trapInfo[idx1].ipAddress == ntohl(inet_addr(pIpAddress))) 
                    && (snmpConfInfo.trapInfo[idx1].port == atoi(pTrapPort))
                    &&(strncmp(pCommunity, snmpConfInfo.trapInfo[idx1].community, SNMP_COMMUNITY_LEN) == 0))
            {
                    return CMD_SUCCESS;
            }

            /*(3) ��Ϣ��һ�£�������Ϣ*/
            memset(&snmpConfInfo.trapInfo[idx1], 0, sizeof(snmpConfInfo.trapInfo[idx1]));
            snprintf(snmpConfInfo.trapInfo[idx1].hostName, SNMP_TRAP_HOST_NAME_LEN+1, "%s", pHostName);      
            snmpConfInfo.trapInfo[idx1].ipAddress = htonl(inet_addr(pIpAddress));
            snmpConfInfo.trapInfo[idx1].port = atoi(pTrapPort);
            snprintf(snmpConfInfo.trapInfo[idx1].community, SNMP_COMMUNITY_LEN+1, "%s", pCommunity);
            tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);               
            vty_out_line(vty, "  The trap host name has existed, and has been modified to current one");
            return CMD_SUCCESS;
        }        
    }

    /*hostName������ʱ�����ִ���*/
    for(idx1 = 0; idx1 < MAX_TRAP; idx1++)
    {
        if(!snmpConfInfo.trapInfo[idx1].hostName[0])
            continue;

        /*(1) ip ��port �ѱ���ͬ��hostNameʹ�ã�������ʾ��Ϣ*/
        if((snmpConfInfo.trapInfo[idx1].ipAddress == ntohl(inet_addr(pIpAddress))) 
                && (snmpConfInfo.trapInfo[idx1].port == atoi(pTrapPort)))
        {            
            if(strncmp(pHostName, snmpConfInfo.trapInfo[idx1].hostName, SNMP_TRAP_HOST_NAME_LEN) != 0)
            {
                vty_out_line(vty, "  Error: The trap IP and port has been used by other host name!");
                return CMD_ERR_NOTHING_TODO;
            }
        }       
    }

     for(emptyIdx = 0; emptyIdx < MAX_TRAP; emptyIdx++)
     {
         if(!snmpConfInfo.trapInfo[emptyIdx].hostName[0])
                 break;
     }   
     if(emptyIdx == MAX_TRAP)     /*(2) trap number�Ѵﵽ���������������ʾ��Ϣ*/
     {
         vty_out_line(vty, "  Error: The trap number reaches the upper limit");
         return CMD_ERR_NOTHING_TODO;
     }
     else       /*(3) ��������Ŀ*/
     {
         memset(&snmpConfInfo.trapInfo[emptyIdx], 0, sizeof(snmpConfInfo.trapInfo[emptyIdx]));
         snprintf(snmpConfInfo.trapInfo[emptyIdx].hostName, SNMP_TRAP_HOST_NAME_LEN+1, "%s", pHostName);      
         snmpConfInfo.trapInfo[emptyIdx].ipAddress = htonl(inet_addr(pIpAddress));
         snmpConfInfo.trapInfo[emptyIdx].port = atoi(pTrapPort);
         snprintf(snmpConfInfo.trapInfo[emptyIdx].community, SNMP_COMMUNITY_LEN+1, "%s", pCommunity);
         tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);
     }
    
      return 0;
}


static int
_tf_snmp_trap_del(
                struct vty *vty,
                const char *pHostName)
{
    int idx;

    if(strlen(pHostName) > SNMP_TRAP_HOST_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The trap host name length is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0; idx < MAX_TRAP; idx++)
    {
        if(!snmpConfInfo.trapInfo[idx].hostName[0])
            continue;

        if(strncmp(pHostName, snmpConfInfo.trapInfo[idx].hostName, SNMP_TRAP_HOST_NAME_LEN) == 0)
        {
            memset(&snmpConfInfo.trapInfo[idx], 0, sizeof(snmpConfInfo.trapInfo[idx]));
            tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);
            
            return CMD_SUCCESS;
        }
    }

    vty_out_line(vty, "  Error: The trap does not exist");
    
    return CMD_ERR_NOTHING_TODO;
}


static int
_tf_snmp_trap_show(
                struct vty *vty)
{    
    int idx, count=0;
    char ipBuf[20];
    
     for(idx = 0; idx < MAX_TRAP; idx++)
     {
         if(!snmpConfInfo.trapInfo[idx].hostName[0])
             continue;
    
        if(!count)
        {
            vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            vty_out_line(vty, "  Index  Host-Name    IP-Address       Port    Community-Name");
            vty_print_string_line(vty, " ", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
        }

        snprintf(ipBuf, sizeof(ipBuf),"%d.%d.%d.%d",
                (snmpConfInfo.trapInfo[idx].ipAddress>>24)&0xFF,
                (snmpConfInfo.trapInfo[idx].ipAddress>>16)&0xFF,
                (snmpConfInfo.trapInfo[idx].ipAddress>>8)&0xFF,
                (snmpConfInfo.trapInfo[idx].ipAddress)&0xFF);

        vty_out_line(vty, "  %-4d   %-12s %-16s %-4d     %-32s",
                count+1,
                snmpConfInfo.trapInfo[idx].hostName,
                ipBuf,
                snmpConfInfo.trapInfo[idx].port,                
                snmpConfInfo.trapInfo[idx].community);

        count += 1;
    }

    if(count)
        vty_print_string_line (vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
    else
        vty_out_line(vty, "  Error: There is no trap exist");

    return CMD_SUCCESS;
}


static int
_tf_snmp_community_set(
                struct vty *vty,
                const char *pType,
                const char *pCommunity)
{
    int idx, emptyIdx;
    int type;

    if(strlen(pCommunity) > SNMP_COMMUNITY_LEN)
    {
        vty_out_line(vty, "  Error: The community length is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    switch(pType[0])
    {
        case 'r':
            type = SNMP_RIGHT_RO;
            break;
        case 'w':
            type = SNMP_RIGHT_RW;
            break;
        default:
            vty_out_line(vty, "  Error: The community type is unknown");
            return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0, emptyIdx = MAX_COMMUNITY; idx < MAX_COMMUNITY; idx++)
    {
        if(!snmpConfInfo.snmpCommunity[idx].community[0])
        {
            if(emptyIdx == MAX_COMMUNITY)
                emptyIdx = idx;
            continue;
        }

        if(strncmp(pCommunity, snmpConfInfo.snmpCommunity[idx].community, SNMP_COMMUNITY_LEN) == 0)
        {
            if(snmpConfInfo.snmpCommunity[idx].type == type)
                return CMD_SUCCESS;
            
            break;
        }
    }

    if(idx == MAX_COMMUNITY && emptyIdx == MAX_COMMUNITY)
    {
        vty_out_line(vty, "  Error: The community number reaches the upper limit");
        return CMD_ERR_NOTHING_TODO;
    }
    else if(idx == MAX_COMMUNITY)
        idx = emptyIdx;
    else
        vty_out_line(vty, "  The community name has existed, and access right has been modified to current one");
    
    memset(&snmpConfInfo.snmpCommunity[idx], 0, sizeof(snmpConfInfo.snmpCommunity[idx]));
    snmpConfInfo.snmpCommunity[idx].type = type;
    snprintf(snmpConfInfo.snmpCommunity[idx].community, SNMP_COMMUNITY_LEN+1, "%s", pCommunity);
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return 0;
}


static int
_tf_snmp_community_del(
                struct vty *vty,
                const char *pCommunity)
{
    int idx;

    if(strlen(pCommunity) > SNMP_COMMUNITY_LEN)
    {
        vty_out_line(vty, "  Error: The community length is too long");
        return CMD_ERR_NOTHING_TODO;
    }


    for(idx = 0; idx < MAX_COMMUNITY; idx++)
    {
        if(strncmp(pCommunity, snmpConfInfo.snmpCommunity[idx].community, SNMP_COMMUNITY_LEN) == 0)
        {
            memset(&snmpConfInfo.snmpCommunity[idx], 0, sizeof(snmpConfInfo.snmpCommunity[idx]));
            tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);
            
            return CMD_SUCCESS;
        }
    }

    vty_out_line(vty, "  Error: The community does not exist");
    
    return CMD_ERR_NOTHING_TODO;
}


static int
_tf_snmp_community_show(
                struct vty *vty,
                const char *pType)
{
    int idx;
    int type;
    int count = 0;

    if(NULL != pType)
    {
        switch(pType[0])
        {
            case 'r':
                type = SNMP_RIGHT_RO;
                break;
            case 'w':
                type = SNMP_RIGHT_RW;
                break;
            default:
                vty_out_line(vty, "  Error: The community type is unknown");
                return CMD_ERR_NOTHING_TODO;
        }
    }

    for(idx = 0; idx < MAX_COMMUNITY; idx++)
    {
        if(!snmpConfInfo.snmpCommunity[idx].community[0])
            continue;
        
        if(NULL != pType && type != snmpConfInfo.snmpCommunity[idx].type)
            continue;
        
        if(!count)
        {
            vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            vty_out_line(vty, "  Community-Name                   VACM-Name        View-Name");
            vty_print_string_line(vty, " ", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
        }

        vty_out_line(vty, "  %-32s %-16s %-16s",
                snmpConfInfo.snmpCommunity[idx].community,
                "default",
                "all");

        count += 1;
    }

    if(count)
        vty_print_string_line (vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
    else
        vty_out_line(vty, "  Error: There is no community exist");

    return CMD_SUCCESS;
}


static int
_tf_snmp_usm_user_set(
                struct vty *vty,
                const char *pUser,
                const char *pGroup,
                const char *pAuthKey,
                const char *pPrivKey)
{

    int idx, emptyIdx;
    int userLen    = 0;
    int authKeyLen = 0;
    int privKeyLen = 0;
    int groupLen   = 0;

    if(pUser != NULL)
        userLen     = strlen(pUser);
    if(pAuthKey != NULL)
        authKeyLen  = strlen(pAuthKey);
    if(pPrivKey != NULL)
        privKeyLen  = strlen(pPrivKey);
    if(pGroup != NULL)
        groupLen    = strlen(pGroup);

    if(userLen != 0 && userLen > SNMP_USM_USER_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The length of user name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(authKeyLen != 0 &&
        (authKeyLen < SNMP_USM_KEY_MIN_LEN || authKeyLen > SNMP_USM_AUTH_KEY_LEN))
    {
        vty_out_line(vty, "  Error: Invalid authentication key");
        return CMD_ERR_NOTHING_TODO;
    }

    if(privKeyLen != 0 &&
        (privKeyLen < SNMP_USM_KEY_MIN_LEN || privKeyLen > SNMP_USM_PRIV_KEY_LEN))
    {
        vty_out_line(vty, "  Error: Invalid privacy key");
        return CMD_ERR_NOTHING_TODO;
    }

    if(groupLen != 0 && groupLen > SNMP_USER_GROUP_LEN)
    {
        vty_out_line(vty, "  Error: The length of group name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(strncmp(pGroup, snmpConfInfo.access[idx].group, SNMP_USER_GROUP_LEN) == 0)
            break;
    }
    if(idx == MAX_ACCESS)
    {
        vty_out_line(vty, "  Error: The group does not exist");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0, emptyIdx = MAX_USM_USER; idx < MAX_USM_USER; idx++)
    {
        if(!snmpConfInfo.usmUserInfo[idx].name[0])
        {
            if(emptyIdx == MAX_USM_USER)
                emptyIdx = idx;
            continue;
        }

        if(strncmp(pUser, snmpConfInfo.usmUserInfo[idx].name, SNMP_USM_USER_NAME_LEN) == 0)
            break;
    }

    if(idx == MAX_USM_USER && emptyIdx == MAX_USM_USER)
    {
        vty_out_line(vty, "  Error: The user number reaches the upper limit");
        return CMD_ERR_NOTHING_TODO;
    }
    else if(idx == MAX_USM_USER)
        idx = emptyIdx;

    memset(&snmpConfInfo.usmUserInfo[idx], 0, sizeof(snmpConfInfo.usmUserInfo[idx]));

    snprintf(snmpConfInfo.usmUserInfo[idx].name, SNMP_USM_USER_NAME_LEN+1, "%s", pUser);

    if(authKeyLen)
        snprintf(snmpConfInfo.usmUserInfo[idx].authpass, SNMP_USM_AUTH_KEY_LEN+1, "%s", pAuthKey);

    if(privKeyLen)
        snprintf(snmpConfInfo.usmUserInfo[idx].privpass, SNMP_USM_PRIV_KEY_LEN+1, "%s", pPrivKey);

    if(groupLen)
        snprintf(snmpConfInfo.usmUserInfo[idx].group, SNMP_USER_GROUP_LEN+1, "%s", pGroup);

    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}


static int
_tf_snmp_usm_user_del(
                    struct vty *vty,
                    const char *pcUser)
{
    int idx;

    if(strlen(pcUser) > SNMP_USM_USER_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The length of the user name is too long");
        return CMD_ERR_NOTHING_TODO;
    }


    for(idx = 0; idx < MAX_USM_USER; idx++)
    {
        if(strncmp(pcUser, snmpConfInfo.usmUserInfo[idx].name, SNMP_USM_USER_NAME_LEN) == 0)
        {
            memset(&snmpConfInfo.usmUserInfo[idx], 0, sizeof(snmpConfInfo.usmUserInfo[idx]));
            tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);
            
            return CMD_SUCCESS;
        }
    }

    vty_out_line(vty, "  Error: The user does not exist");
    
    return CMD_ERR_NOTHING_TODO;
}


static int
_tf_snmp_usm_user_show(
                struct vty *vty,
                const char *pcUser)
{
    int idx;

    if((pcUser == NULL) || (strlen(pcUser) > SNMP_USM_USER_NAME_LEN))
    {
        vty_out_line(vty, "  Error: The user name is invalid");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0; idx < MAX_USM_USER; idx++)
    {
        if(strncmp(pcUser, snmpConfInfo.usmUserInfo[idx].name, SNMP_USM_USER_NAME_LEN) == 0)
        {
            vty_out_line(vty,           "  User name            : %s", 
                            snmpConfInfo.usmUserInfo[idx].name);
            vty_out_line(vty,           "  Group name           : %s", 
                            snmpConfInfo.usmUserInfo[idx].group);
            if(snmpConfInfo.usmUserInfo[idx].authpass[0])
            {
                vty_out_line(vty,       "  Authentication mode  : md5");
                vty_out_line(vty,       "  Authentication key   : %s", snmpConfInfo.usmUserInfo[idx].authpass);

                if(snmpConfInfo.usmUserInfo[idx].privpass[0])
                {
                    vty_out_line(vty,   "  Privacy mode         : des56");
                    vty_out_line(vty,   "  Privacy key          : %s", snmpConfInfo.usmUserInfo[idx].privpass);
                }
                else
                    vty_out_line(vty,   "  Privacy mode         : no privacy mode");
            }
            else
            {
                vty_out_line(vty,       "  Authentication mode  : no authentication mode");
                vty_out_line(vty,       "  Privacy mode         : no privacy mode");
            }

            return CMD_SUCCESS;
        }
    }
    vty_out_line(vty, "  Error: The user does not exist");
    
    return CMD_ERR_NOTHING_TODO;
}


static int
_tf_snmp_usm_user_show_all(
                struct vty *vty)
{
    int idx;
    int count = 0;

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(snmpConfInfo.usmUserInfo[idx].name[0])
        {
            vty_out_line(vty,           "  User name            : %s", 
                            snmpConfInfo.usmUserInfo[idx].name);
            vty_out_line(vty,           "  Group name           : %s", 
                            snmpConfInfo.usmUserInfo[idx].group);
            if(snmpConfInfo.usmUserInfo[idx].authpass[0])
            {
                vty_out_line(vty,       "  Authentication mode  : md5");
                vty_out_line(vty,       "  Authentication key   : %s", snmpConfInfo.usmUserInfo[idx].authpass);

                if(snmpConfInfo.usmUserInfo[idx].privpass[0])
                {
                    vty_out_line(vty,   "  Privacy mode         : des56");
                    vty_out_line(vty,   "  Privacy key          : %s", snmpConfInfo.usmUserInfo[idx].privpass);
                }
                else
                    vty_out_line(vty,   "  Privacy mode         : no privacy mode");
            }
            else
            {
                vty_out_line(vty,       "  Authentication mode  : no authentication mode");
                vty_out_line(vty,       "  Privacy mode         : no privacy mode");
            }
            
            vty_out_line(vty, "  ");
            count += 1;
        }
    }

    if(count)
        vty_out_line(vty, "  Total number : %d", count);
    else
        vty_out_line(vty, "  Error: There is no user exist");

    return CMD_SUCCESS;
}


static int
_tf_snmp_access_set(
                    struct vty *vty,
                    const char *pGroup,
                    const char *pSecLevel,
                    const char *pReadView,
                    const char *pWriteView,
                    const char *pNotifyView)
{
    int idx, emptyIdx;
    int groupLen           = 0;
    int secLevLen          = 0;
    int readViewLen        = 0;
    int writeViewLen       = 0;
    int NotifyViewLen      = 0;
    char acSecureLevel[32] = {0};
    groupLen    = strlen(pGroup);
    secLevLen   = strlen(pSecLevel);

    if(pReadView != NULL)
        readViewLen = strlen(pReadView);

    if(pWriteView != NULL)
        writeViewLen = strlen(pWriteView);

    if(pNotifyView != NULL)
        NotifyViewLen = strlen(pNotifyView);

    if(groupLen != 0 && groupLen > SNMP_USER_GROUP_LEN)
    {
        vty_out_line(vty, "  Error: The length of the group name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(secLevLen != 0 && secLevLen > SNMP_SEC_LEVEL_LEN)
    {
        vty_out_line(vty, "  Error: The length of the secure level is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(pSecLevel[0])
    {
        switch ( pSecLevel[0] )
        {
            case 'a' :
                strcpy(acSecureLevel, "auth");
                break;
            case 'n' :
                strcpy(acSecureLevel, "noauth");
                break;
            case 'p' :
                strcpy(acSecureLevel, "priv");
                break;
            default:
                vty_out_line(vty, "  Error: The secure level is unknown");
                return CMD_ERR_NOTHING_TODO;
        }
    }

    if(writeViewLen != 0 && writeViewLen > SNMP_VIEW_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The length of readView name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(writeViewLen != 0 && writeViewLen > SNMP_VIEW_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The length of writeView name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    if(NotifyViewLen != 0 && NotifyViewLen > SNMP_VIEW_NAME_LEN)
    {
        vty_out_line(vty, "  Error: The length of NotifyView name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0, emptyIdx = MAX_ACCESS; idx < MAX_ACCESS; idx++)
    {
        if(!snmpConfInfo.access[idx].group[0])
        {
            if(emptyIdx == MAX_ACCESS)
                emptyIdx = idx;
            continue;
        }

        if(strncmp(pGroup, snmpConfInfo.access[idx].group, SNMP_USER_GROUP_LEN) == 0)
            break;
    }

    if(idx == MAX_ACCESS && emptyIdx == MAX_ACCESS)
    {
        vty_out_line(vty, "  Error: The number of group reaches the upper limit");
        return CMD_ERR_NOTHING_TODO;
    }
    else if(idx == MAX_ACCESS)
        idx = emptyIdx;

    memset(&snmpConfInfo.access[idx], 0, sizeof(snmpConfInfo.usmUserInfo[idx]));

    snprintf(snmpConfInfo.access[idx].group, SNMP_USER_GROUP_LEN+1, "%s", pGroup);
    snprintf(snmpConfInfo.access[idx].secLevel, SNMP_SEC_LEVEL_LEN+1, "%s", acSecureLevel);

    if(readViewLen)
        snprintf(snmpConfInfo.access[idx].readView, SNMP_VIEW_NAME_LEN+1, "%s", pReadView);
    else
        snprintf(snmpConfInfo.access[idx].readView, SNMP_VIEW_NAME_LEN+1, "%s", "none");

    if(writeViewLen)
        snprintf(snmpConfInfo.access[idx].writeView, SNMP_VIEW_NAME_LEN+1, "%s", pWriteView);
    else
        snprintf(snmpConfInfo.access[idx].writeView, SNMP_VIEW_NAME_LEN+1, "%s", "none");

    if(NotifyViewLen)
        snprintf(snmpConfInfo.access[idx].notifyView, SNMP_VIEW_NAME_LEN+1, "%s", pNotifyView);
    else
        snprintf(snmpConfInfo.access[idx].notifyView, SNMP_VIEW_NAME_LEN+1, "%s", "none");

    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}


static int
_tf_snmp_access_del(
                    struct vty *vty,
                    const char *pGroup)
{
    int idx, i;
    int groupLen = strlen(pGroup);
    
    if(groupLen != 0 && groupLen > SNMP_USER_GROUP_LEN)
    {
        vty_out_line(vty, "  Error: The length of the group name is too long");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(strncmp(pGroup, snmpConfInfo.access[idx].group, SNMP_USER_GROUP_LEN) != 0)
            continue;

        for(i = 0; i < MAX_USM_USER; i++)
        {
            if(strncmp(pGroup, snmpConfInfo.usmUserInfo[i].group, SNMP_USER_GROUP_LEN) == 0)
            {
                vty_out_line(vty, "  Error: The group has been used by user \"%s\"", snmpConfInfo.usmUserInfo[i].name);
                return CMD_ERR_NOTHING_TODO;
            }
        }

        memset(&(snmpConfInfo.access[idx]), 0, sizeof(snmpConfInfo.access[idx]));
        tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);
        
        return CMD_SUCCESS;
    }

    vty_out_line(vty, "  Error: The group does not exist");

    return CMD_ERR_NOTHING_TODO;
}


static int
_tf_snmp_access_show(
                struct vty *vty,
                const char *pGroup)
{
    int idx;

    if((pGroup == NULL) || (strlen(pGroup) > SNMP_USER_GROUP_LEN))
    {
        vty_out_line(vty, "  The group name is invalid");
        return CMD_ERR_NOTHING_TODO;
    }

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(strncmp(pGroup, snmpConfInfo.access[idx].group, SNMP_USER_GROUP_LEN) == 0)
        {
            vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            vty_out_line(vty, "  Group                            Secure     Read   Write  Notify");
            vty_out_line(vty, "  Name                             Level      View   View   View");
            vty_print_string_line(vty, " ", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            vty_out_line(vty, "  %-32s %-10s %-6s %-6s %-6s ",
                                snmpConfInfo.access[idx].group,
                                snmpConfInfo.access[idx].secLevel,
                                snmpConfInfo.access[idx].readView,
                                snmpConfInfo.access[idx].writeView,
                                snmpConfInfo.access[idx].notifyView);
            vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            return CMD_SUCCESS;
        }
    }

    vty_out_line(vty, "  Error: The group does not exist");

    return CMD_ERR_NOTHING_TODO;
}


int
_tf_snmp_access_show_all(
                struct vty *vty)
{
    int idx;
    int count = 0;

    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(snmpConfInfo.access[idx].group[0])
        {
            if(count == 0)
            {
                vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
                vty_out_line(vty, "  Group                            Secure     Read   Write  Notify");
                vty_out_line(vty, "  Name                             Level      View   View   View");
                vty_print_string_line(vty, " ", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
            }

            vty_out_line(vty, "  %-32s %-10s %-6s %-6s %-6s ",
                                snmpConfInfo.access[idx].group,
                                snmpConfInfo.access[idx].secLevel,
                                snmpConfInfo.access[idx].readView,
                                snmpConfInfo.access[idx].writeView,
                                snmpConfInfo.access[idx].notifyView);
            count += 1;
        }

    }

    if(count)
        vty_print_string_line(vty, "", __PROMPT_SEPARATOR_LENGTH__, __PROMPT_SEPARATOR_CHAR__);
    else
        vty_out_line(vty, "  Error: There is no group exist");

    return CMD_SUCCESS;
}


/* snmp config */
int
tf_snmp_config_write(void)
{
    int idx = 0;
    char line[512];
    int len = 0;

    snprintf(line, sizeof(line), "# snmp config");
    vtysh_config_parse_line(line);

    /* snmp-agent sys-info */
    if(snmpConfInfo.sysInfo.sysName[0])
    {
        snprintf(line, sizeof(line), "snmp-agent sys-info name %s", snmpConfInfo.sysInfo.sysName);
        vtysh_config_parse_line(line);
    }

    if(snmpConfInfo.sysInfo.sysDesc[0])
    {
        snprintf(line, sizeof(line), "snmp-agent sys-info description %s", snmpConfInfo.sysInfo.sysDesc);
        vtysh_config_parse_line(line);
    }
    
    if(snmpConfInfo.sysInfo.sysContact[0])
    {
        snprintf(line, sizeof(line), "snmp-agent sys-info contact %s", snmpConfInfo.sysInfo.sysContact);
        vtysh_config_parse_line(line);
    }

    if(snmpConfInfo.sysInfo.sysLocation[0])
    {
        snprintf(line, sizeof(line), "snmp-agent sys-info location %s", snmpConfInfo.sysInfo.sysLocation);
        vtysh_config_parse_line(line);
    }

    /* snmp-agent community */
    for(idx = 0; idx < MAX_COMMUNITY; idx++)
    {
        if(snmpConfInfo.snmpCommunity[idx].community[0])
        {
            if(SNMP_RIGHT_RO == snmpConfInfo.snmpCommunity[idx].type)
                snprintf(line, sizeof(line), "snmp-agent community read %s",
                                snmpConfInfo.snmpCommunity[idx].community);
            else if(SNMP_RIGHT_RW == snmpConfInfo.snmpCommunity[idx].type)
                snprintf(line, sizeof(line), "snmp-agent community write %s",
                                snmpConfInfo.snmpCommunity[idx].community);
            else
                continue;
            vtysh_config_parse_line(line);
        }
    }

    /* snmp-agent trap */
    for(idx = 0; idx < MAX_TRAP; idx++)
    {
        if(!snmpConfInfo.trapInfo[idx].hostName[0])
           continue;

        snprintf(line, sizeof(line), "snmp-agent trap %s %d.%d.%d.%d %d %s",
                    snmpConfInfo.trapInfo[idx].hostName,
                    (snmpConfInfo.trapInfo[idx].ipAddress>>24)&0xFF,
                    (snmpConfInfo.trapInfo[idx].ipAddress>>16)&0xFF,
                    (snmpConfInfo.trapInfo[idx].ipAddress>>8)&0xFF,
                    (snmpConfInfo.trapInfo[idx].ipAddress)&0xFF,
                    snmpConfInfo.trapInfo[idx].port,
                    snmpConfInfo.trapInfo[idx].community);  

        vtysh_config_parse_line(line);
    }

    /* access */
    for(idx = 0; idx < MAX_ACCESS; idx++)
    {
        if(snmpConfInfo.access[idx].group[0])
        {
            /* group and secure level */
            if(snmpConfInfo.access[idx].secLevel[0] == 'n')
                len = snprintf(line, sizeof(line), "snmp-agent group v3 %s noauth ",
                                        snmpConfInfo.access[idx].group);
            else if(snmpConfInfo.access[idx].secLevel[0] == 'a')
                len = snprintf(line, sizeof(line), "snmp-agent group v3 %s authentication ",
                                        snmpConfInfo.access[idx].group);
            else if(snmpConfInfo.access[idx].secLevel[0] == 'p')
                len = snprintf(line, sizeof(line), "snmp-agent group v3 %s privacy ",
                                        snmpConfInfo.access[idx].group);
            else
                continue;

            /* view */
            if(snmpConfInfo.access[idx].readView)
                len += snprintf(line + len, sizeof(line) - len, "read-view %s ",
                                                snmpConfInfo.access[idx].readView);
            if(snmpConfInfo.access[idx].writeView)
                len += snprintf(line + len, sizeof(line) - len, "write-view %s ",
                                                snmpConfInfo.access[idx].writeView);
            if(snmpConfInfo.access[idx].notifyView)
                len += snprintf(line + len, sizeof(line) - len, "notify-view %s ",
                                                snmpConfInfo.access[idx].notifyView);

            vtysh_config_parse_line(line);
        }
    }

    /* snmp-agent usm-user*/
    for(idx = 0; idx < MAX_USM_USER; idx++)
    {

        if(snmpConfInfo.usmUserInfo[idx].name[0])
        {
            len = snprintf(line, sizeof(line), "snmp-agent usm-user v3 %s %s",
                            snmpConfInfo.usmUserInfo[idx].name,
                            snmpConfInfo.usmUserInfo[idx].group);

            if(snmpConfInfo.usmUserInfo[idx].authpass[0])
            {
                len += snprintf(line + len, sizeof(line) - len, " authentication-mode md5 %s ",
                            snmpConfInfo.usmUserInfo[idx].authpass);
            }
            if(snmpConfInfo.usmUserInfo[idx].privpass[0])
            {
                len += snprintf(line + len, sizeof(line) - len, " privacy-mode des56  %s ",
                            snmpConfInfo.usmUserInfo[idx].privpass);
            }

            vtysh_config_parse_line(line);
        }
    }

    if(snmpConfInfo.sysInfo.enable != SNMP_DEF_ENABLE)
    {
        if(snmpConfInfo.sysInfo.enable)
            snprintf(line, sizeof(line), "snmp-agent enable");
        else
            snprintf(line, sizeof(line), "snmp-agent disable");
        vtysh_config_parse_line(line);
    }

    return 0;
}



/*  snmp-agent sys-info
        (contact CONTACT | location LOCATION | version ({ v1 | v2c | v3 } *| all ))
    no snmp-agent sys-info ( contact | location )
    no snmp-agent sys-info version ( ( v1 | v2c | v3 ) | all )
*/
#if DEF_FUNC("snmp-agent sys-info")

DEFUN (
    vty_snmp_name_set,
    vty_snmp_name_set_cmd,
    "snmp-agent sys-info name .NAME",
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Name of the system\n"
    "Name of the system. <S><Length 1-"SNMP_SYS_NAME_LEN_STR">\n")
{
    int len, idx;
    char buf[SNMP_SYS_NAME_LEN+1];

    for(len = 0, idx = 0; idx < argc; idx++)
    {
        if(idx == 0)
            len += snprintf(buf + len, sizeof(buf) - len, "%s", argv[idx]);
        else
            len += snprintf(buf + len, sizeof(buf) - len, " %s", argv[idx]);
        if(len > SNMP_SYS_NAME_LEN)
        {
            vty_out_line(vty, "  Error: The length of the name is too long");
            return CMD_ERR_NOTHING_TODO;
        } 
    }
    
    snprintf(snmpConfInfo.sysInfo.sysName, SNMP_SYS_NAME_LEN+1, "%s", buf);
    
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_desc_set,
    vty_snmp_desc_set_cmd,
    "snmp-agent sys-info description .DESCRIPTION",
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Description of the system\n"
    "Description of the system. <S><Length 1-"SNMP_SYS_DESC_LEN_STR">\n")
{
    int len, idx;
    char buf[SNMP_SYS_DESC_LEN+1];

    for(len = 0, idx = 0; idx < argc; idx++)
    {
        if(idx == 0)
            len += snprintf(buf + len, sizeof(buf) - len, "%s", argv[idx]);
        else
            len += snprintf(buf + len, sizeof(buf) - len, " %s", argv[idx]);
        if(len > SNMP_SYS_DESC_LEN)
        {
            vty_out_line(vty, "  Error: The length of the description is too long");
            return CMD_ERR_NOTHING_TODO;
        } 
    }
    
    snprintf(snmpConfInfo.sysInfo.sysDesc, SNMP_SYS_DESC_LEN+1, "%s", buf);
    
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_contact_set,
    vty_snmp_contact_set_cmd,
    "snmp-agent sys-info contact .CONTACT",
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Contact information of system maintenance\n"
    "Contact information. <S><Length 1-"SNMP_CONTACT_LEN_STR">\n")
{
    int len, idx;
    char buf[SNMP_CONTACT_LEN+1];

    for(len = 0, idx = 0; idx < argc; idx++)
    {
        if(idx == 0)
            len += snprintf(buf + len, sizeof(buf) - len, "%s", argv[idx]);
        else
            len += snprintf(buf + len, sizeof(buf) - len, " %s", argv[idx]);
        if(len > SNMP_CONTACT_LEN)
        {
            vty_out_line(vty, "  Error: The length of the contact is too long");
            return CMD_ERR_NOTHING_TODO;
        } 
    }
    
    snprintf(snmpConfInfo.sysInfo.sysContact, SNMP_CONTACT_LEN+1, "%s", buf);
    
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_location_set,
    vty_snmp_location_set_cmd,
    "snmp-agent sys-info location .LOCATION",
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Address information of system maintenance\n"
    "Address information. <S><Length 1-"SNMP_LOCATION_LEN_STR">\n")
{
    int len, idx;
    char buf[SNMP_LOCATION_LEN+1];

    for(len = 0, idx = 0; idx < argc; idx++)
    {
        if(idx == 0)
            len += snprintf(buf + len, sizeof(buf) - len, "%s", argv[idx]);
        else
            len += snprintf(buf + len, sizeof(buf) - len, " %s", argv[idx]);
        if(len > SNMP_LOCATION_LEN)
        {
            vty_out_line(vty, "  Error: The length of the location is too long");
            return CMD_ERR_NOTHING_TODO;
        } 
    }
    
    snprintf(snmpConfInfo.sysInfo.sysLocation, SNMP_LOCATION_LEN+1, "%s", buf);

    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_name_del,
    vty_snmp_name_del_cmd,
    "no snmp-agent sys-info name",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Name of the system\n")
{
    memset(snmpConfInfo.sysInfo.sysName, 0, SNMP_SYS_NAME_LEN);
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_desc_del,
    vty_snmp_desc_del_cmd,
    "no snmp-agent sys-info description",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Description of the system\n")
{
    memset(snmpConfInfo.sysInfo.sysDesc, 0, SNMP_SYS_DESC_LEN);
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_contact_del,
    vty_snmp_contact_del_cmd,
    "no snmp-agent sys-info contact",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Contact information of system maintenance\n")
{
    memset(snmpConfInfo.sysInfo.sysContact, 0, SNMP_CONTACT_LEN);
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_location_del,
    vty_snmp_location_del_cmd,
    "no snmp-agent sys-info location",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE
    "Address information of system maintenance\n")
{
    memset(snmpConfInfo.sysInfo.sysLocation, 0, SNMP_LOCATION_LEN);
    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 1);

    return CMD_SUCCESS;
}


/*
    show snmp-agent sys-info [ contact | location ]
*/


DEFUN (
    vty_snmp_sys_info_show,
    vty_snmp_sys_info_show_cmd,
    "show snmp-agent sys-info",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_SYS_INFO_NODE)
{
    vty_out_line(vty, "  The name of this managed node:");
    if(snmpConfInfo.sysInfo.sysName[0])
        vty_out_line(vty, "      %s", snmpConfInfo.sysInfo.sysName);
    else
        vty_out_line(vty, "      "SNMP_DEF_SYS_NAME);
    vty_out_line(vty, " ");

    vty_out_line(vty, "  The description of this managed node:");
    if(snmpConfInfo.sysInfo.sysDesc[0])
        vty_out_line(vty, "      %s", snmpConfInfo.sysInfo.sysDesc);
    else
        vty_out_line(vty, "      "SNMP_DEF_SYS_DESC);
    vty_out_line(vty, " ");

    vty_out_line(vty, "  The contact person for this managed node:");
    if(snmpConfInfo.sysInfo.sysContact[0])
        vty_out_line(vty, "      %s", snmpConfInfo.sysInfo.sysContact);
    else
        vty_out_line(vty, "      "SNMP_DEF_CONTACT);
    vty_out_line(vty, " ");

    vty_out_line(vty, "  The physical location of this node:");
    if(snmpConfInfo.sysInfo.sysLocation[0])
        vty_out_line(vty, "      %s", snmpConfInfo.sysInfo.sysLocation);
    else
        vty_out_line(vty, "      "SNMP_DEF_LOCATION);

    return CMD_SUCCESS;

}

#endif

/*
    snmp-agent
    no snmp-agent
*/
#if DEF_FUNC("snmp-agent")

DEFUN (
    vty_snmp_agent_set,
    vty_snmp_agent_set_cmd,
    "snmp-agent (disable|enable)",
    DESC_SNMP_MODULE
    "Disable snmp agent\n"
    "Enable snmp agent\n")
{
    switch(argv[0][0])
    {
        case 'e':
            _tf_snmp_start(vty);
            break;
        case 'd':
            _tf_snmp_stop(vty);
            break;
    }

    return CMD_SUCCESS;
}


DEFUN (
    vty_snmp_state_show,
    vty_snmp_state_show_cmd,
    "show snmp-agent status",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_STATE_NODE)
{
    vty_out_line(vty, "  Snmp agent status: %s", 
                    snmpConfInfo.sysInfo.enable ? "Enable" : "Disable");

    return CMD_SUCCESS;
}

#endif

#if DEF_FUNC("snmp-agent trap")
DEFUN (
    vty_snmp_trap_set,
    vty_snmp_trap_set_cmd,
    "snmp-agent trap HOST-NAME A.B.C.D <1-65535> COMMUNITY-NAME",
    DESC_SNMP_MODULE
    "Snmp trap\n"
    "Host name. <S><Length 1-"SNMP_TRAP_HOST_NAME_LEN_STR">\n"
    "Snmp trap IP address\n"
    "Snmp trap port\n"
    "Community name. <S><Length 1-"SNMP_COMMUNITY_LEN_STR">\n")
{
    _tf_snmp_trap_set(vty, argv[0], argv[1], argv[2], argv[3]);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_trap_del,
    vty_snmp_trap_del_cmd,
    "no snmp-agent trap HOST-NAME",
    DESC_NO
    DESC_SNMP_MODULE
    "Snmp trap\n"
    "Host name. <S><Length 1-"SNMP_TRAP_HOST_NAME_LEN_STR">\n")
{
    _tf_snmp_trap_del(vty, argv[0]);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_trap_show,
    vty_snmp_trap_show_cmd,
    "show snmp-agent trap",
    DESC_SHOW
    DESC_SNMP_MODULE
    "Snmp trap\n")
{
    _tf_snmp_trap_show(vty);

    return CMD_SUCCESS;
}
#endif

/*
    snmp-agent community
    snmp-agent community ( read | write ) COMMUNITY
    no snmp-agent community COMMUNITY
*/
#if DEF_FUNC("snmp-agent community")

DEFUN (
    vty_snmp_community_set,
    vty_snmp_community_set_cmd,
    "snmp-agent community (read|write) COMMUNITY-NAME ",
    DESC_SNMP_MODULE
    DESC_SNMP_COMMUNITY_NODE
    "Read-only access for this community in the view\n"
    "Read-write access for this community in the view\n"
    "Community name. <S><Length 1-"SNMP_COMMUNITY_LEN_STR">\n")
{
    _tf_snmp_community_set(vty, argv[0], argv[1]);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_community_del,
    vty_snmp_community_del_cmd,
    "no snmp-agent community COMMUNITY-NAME",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_COMMUNITY_NODE
    "Community name. <S><Length 1-"SNMP_COMMUNITY_LEN_STR">\n")
{
    _tf_snmp_community_del(vty, argv[0]);

    return CMD_SUCCESS;
}

/*
    show snmp-agent community (read | write)
*/
DEFUN (
    vty_snmp_community_show,
    vty_snmp_community_show_cmd,
    "show snmp-agent community (read | write)",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_COMMUNITY_NODE
    "Read-only access for this community in the view\n"
    "Read-write access for this community in the view\n")
{
    _tf_snmp_community_show(vty, argv[0]);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_community_show_all,
    vty_snmp_community_show_all_cmd,
    "show snmp-agent community",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_COMMUNITY_NODE)
{
    _tf_snmp_community_show(vty, NULL);

    return CMD_SUCCESS;
}

#endif

/*
   snmp-agent usm-user
   snmp-agent usm-user v3 (read|write) USER GROUP [ authentication-mode ( md5 | sha )
   AUTHKEY [ privacy-mode ( aes128 | des56 ) PRIKEY ] ]
   undo snmp-agent usm-user v3 (read|write) USER
 */
#if DEF_FUNC("snmp-agent usm-user")
DEFUN (
    vty_snmp_usm_user_set,
    vty_snmp_usm_user_set_cmd,
    "snmp-agent usm-user v3 USER GROUP",
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE
    DESC_SNMP_USM_USER_V3_NODE
    "The string of user name. <S><Length 1-"SNMP_USM_USER_NAME_LEN_STR">\n"
    "The string of group to which the specified user belongs.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n")
{
    _tf_snmp_usm_user_set(vty, argv[0], argv[1], NULL, NULL);

    return CMD_SUCCESS;
}


DEFUN (
    vty_snmp_usm_user_set_auth,
    vty_snmp_usm_user_set_auth_cmd,
    "snmp-agent usm-user v3 USER GROUP authentication-mode md5 AUTH-KEY",
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE
    DESC_SNMP_USM_USER_V3_NODE
    "The string of user name. <S><Length 1-"SNMP_USM_USER_NAME_LEN_STR">\n"
    "The string of group to which the specified user belongs.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n"
    "Specify the authentication mode for the user\n"
    "Authenticate with HMAC MD5 algorithm\n"
    "Password of user authentication. <S><Length "SNMP_USM_KEY_MIN_LEN_STR"-"SNMP_USM_AUTH_KEY_LEN_STR">\n")
{
    _tf_snmp_usm_user_set(vty, argv[0], argv[1], argv[2], NULL);

    return CMD_SUCCESS;
}


DEFUN (
    vty_snmp_usm_user_set_privacy,
    vty_snmp_usm_user_set_privacy_cmd,
    "snmp-agent usm-user v3 USER GROUP authentication-mode md5 AUTH-KEY privacy-mode des56 PRI-KEY",
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE
    DESC_SNMP_USM_USER_V3_NODE
    "The string of user name. <S><Length 1-"SNMP_USM_USER_NAME_LEN_STR">\n"
    "The string of group to which the specified user belongs.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n"
    "Specify the authentication mode for the user\n"
    "Authenticate with HMAC MD5 algorithm\n"
    "Password of user authentication. <S><Length "SNMP_USM_KEY_MIN_LEN_STR"-"SNMP_USM_AUTH_KEY_LEN_STR">\n"
    "Specify the privacy mode for the user\n"
    "Use the 56bits DES encryption algorithm\n"
    "Password of user Encryption. <S><Length "SNMP_USM_KEY_MIN_LEN_STR"-"SNMP_USM_PRIV_KEY_LEN_STR">")
{
    _tf_snmp_usm_user_set(vty, argv[0], argv[1], argv[2], argv[3]);

    return CMD_SUCCESS;
}



DEFUN (
    vty_snmp_usm_user_del,
    vty_snmp_usm_user_del_cmd,
    "no snmp-agent usm-user v3 USER",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE
    DESC_SNMP_USM_USER_V3_NODE
    "The string of user name. <S><Length 1-"SNMP_USM_USER_NAME_LEN_STR">\n")
{
    _tf_snmp_usm_user_del(vty, argv[0]);

    return CMD_SUCCESS;
}


/*
    show snmp-agent usm-user [ user-name ]
*/
DEFUN (
    vty_snmp_usm_user_show,
    vty_snmp_usm_user_show_cmd,
    "show snmp-agent usm-user USER",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE
    "The string of user name. <S><Length 1-"SNMP_USM_USER_NAME_LEN_STR">\n")
{
    if(argc)
        _tf_snmp_usm_user_show(vty, argv[0]);
    else
        _tf_snmp_usm_user_show_all(vty);

    return CMD_SUCCESS;
}

ALIAS (
    vty_snmp_usm_user_show,
    vty_snmp_usm_user_show_default_cmd,
    "show snmp-agent usm-user",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_USM_USER_NODE)


#endif

/*
    snmp-agent group
    snmp-agent group v3 GROUP-NAME ( authentication | noauth | privacy )
    [ read-view viewname | write-view viewname | notify-view viewname ]*
*/
#if DEF_FUNC("snmp-agent group")

DEFUN (
    vty_snmp_usm_user_group_set,
    vty_snmp_usm_user_group_set_cmd,
    "snmp-agent group v3 GROUP (authentication | noauth | privacy) {read-view (all | none) | write-view (all | none) | notify-view (all | none)}",
    DESC_SNMP_MODULE
    DESC_SNMP_GROUP_NODE
    DESC_SNMP_GROUP_V3_NODE
    "Group name.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n"
    "Secure level authentication.\n"
    "Secure level noauth.\n"
    "Secure level privacy.\n"
    "Read view.\n"
    "All views.\n"
    "None view.\n"
    "Write view.\n"
    "All views.\n"
    "None view.\n"
    "Notify view.\n"
    "All views.\n"
    "None view.\n")
{
    _tf_snmp_access_set(vty, argv[0], argv[1], argv[2], argv[3], argv[4]);

    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_usm_user_group_default_set,
    vty_snmp_usm_user_group_set_default_cmd,
    "snmp-agent group v3 GROUP (authentication | noauth | privacy)",
    DESC_SNMP_MODULE
    DESC_SNMP_GROUP_NODE
    DESC_SNMP_GROUP_V3_NODE
    "Group name.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n"
    "Secure level authentication.\n"
    "Secure level noauth.\n"
    "Secure level privacy.\n")
{
    _tf_snmp_access_set(vty, argv[0], argv[1], NULL, NULL, NULL);
    
    return CMD_SUCCESS;
}

DEFUN (
    vty_snmp_usm_user_group_del,
    vty_snmp_usm_user_group_del_cmd,
    "no snmp-agent group v3 GROUP",
    DESC_NO
    DESC_SNMP_MODULE
    DESC_SNMP_GROUP_NODE
    DESC_SNMP_GROUP_V3_NODE
    "Group name.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n"
    "Secure level authentication.\n"
    "Secure level noauth.\n"
    "Secure level privacy.\n")
{
    _tf_snmp_access_del(vty, argv[0]);
    
    return CMD_SUCCESS;
}

#endif

/*
    show snmp-agent group [ group-name ]
*/
DEFUN (
    vty_snmp_group_show,
    vty_snmp_group_show_cmd,
    "show snmp-agent group GROUP",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_GROUP_NODE
    "Group name.<S><Length 1-"SNMP_USER_GROUP_LEN_STR">\n")
{
    if(argc)
        _tf_snmp_access_show(vty, argv[0]);
    else
        _tf_snmp_access_show_all(vty);

    return CMD_SUCCESS;
}

ALIAS (
    vty_snmp_group_show,
    vty_snmp_group_show_default_cmd,
    "show snmp-agent group",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_GROUP_NODE)


/* snmp-agent mib-view [ view-name ] */
#if DEF_FUNC("snmp-agent mib-view")
DEFUN (
    vty_snmp_mib_view_show,
    vty_snmp_mib_view_show_cmd,
    "show snmp-agent mib-view",
    DESC_SHOW
    DESC_SNMP_MODULE
    DESC_SNMP_MIB_VIEW_NODE)
{
    vty_out_line(vty, "  View name      : all");
    vty_out_line(vty, "  MIB subtree    : default");
    vty_out_line(vty, "  Subtree mask   : none");
    vty_out_line(vty, "  Storage type   : nonVolatile");
    vty_out_line(vty, "  View type      : included");
    vty_out_line(vty, "  View status    : active");

    return CMD_SUCCESS;

}
#endif


#if DEF_FUNC("cmd install")

void 
tf_vty_snmp_install(void)
{
    /* snmp-agent sys-info */
    install_element (CONFIG_NODE, &vty_snmp_name_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_name_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_desc_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_desc_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_contact_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_contact_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_location_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_location_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_sys_info_show_cmd);

    /* snmp-agent */
    install_element (CONFIG_NODE, &vty_snmp_agent_set_cmd);

    /* snmp-agent trap */
    install_element (CONFIG_NODE, &vty_snmp_trap_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_trap_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_trap_show_cmd);
    
    /* snmp-agent community */
    install_element (CONFIG_NODE, &vty_snmp_community_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_community_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_community_show_cmd);
    install_element (CONFIG_NODE, &vty_snmp_community_show_all_cmd);
    
    /* snmp-agent usm-user */
    install_element (CONFIG_NODE, &vty_snmp_usm_user_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_set_auth_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_set_privacy_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_show_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_show_default_cmd);

    /* snmp-agent group */
    install_element (CONFIG_NODE, &vty_snmp_usm_user_group_set_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_group_set_default_cmd);
    install_element (CONFIG_NODE, &vty_snmp_usm_user_group_del_cmd);
    install_element (CONFIG_NODE, &vty_snmp_group_show_cmd);
    install_element (CONFIG_NODE, &vty_snmp_group_show_default_cmd);

    /* snmp-agent mib-view */
    //install_element (CONFIG_NODE, &vty_snmp_mib_view_show_cmd);

    /* snmp-agent state */
    install_element (CONFIG_NODE, &vty_snmp_state_show_cmd);
    
}

#endif

ULONG process_vtysh_task(char *pMsgOut,ULONG ulMsgLen,UCHAR ucMsgType,UCHAR ucSrcMo)
{
    short rc = 0;
    IPC_APP_MSG *pMsg;
    IPC_EVENT_R_INFO *pEvent;
    char ErrMsg[MAX_ERROR_MSG_LEN] = {0};
    int datalen = 0;

    if (ucMsgType == IPC_EVENT_RELEASE) {
        datalen = ulMsgLen - sizeof(IPC_EVENT_R_INFO);
        if (datalen < 0) {
            //log_print(LOG_DEBUG, "receive bad IPC_EVENT_RELEASE msg!message length %d is less than size of IPC_EVENT_R_INFO:%d.\n",datalen,sizeof(IPC_EVENT_R_INFO));
            goto end;
        }
        pEvent = (IPC_EVENT_R_INFO *)pMsgOut;
        debuginfo("eventid:%d,\n", pEvent->EventMsgHead.ucEventId);
        switch (pEvent->EventMsgHead.ucEventId)
        {        
            default:
                break;
        }
    } else if (ucMsgType == IPC_MSG_CMD) {
        datalen = ulMsgLen - sizeof(IPC_APP_MSG);
        if (datalen < 0){
            //log_print(LOG_DEBUG, "receive bad IPC_MSG_CMD msg!message length %d is less than size of IPC_APP_MSG:%d.\n",datalen,sizeof(IPC_APP_MSG));
            goto end;
        }
        pMsg = (IPC_APP_MSG *)pMsgOut;
        debuginfo("msg cmd,msgid:%d,\n", pMsg->MsgHead.MsgID);
        switch( pMsg->MsgHead.MsgID)
        {
            case IPC_PKT_SNMP_CONFIG_SET_MSG:
            {              
                SNMP_CONF_INFO_STRU *tSnmpConfigInfo=NULL;
                
                if (datalen < sizeof(SNMP_CONF_INFO_STRU)){
                    //log_print(LOG_DEBUG, "receive bad IPC_PKT_SNMP_CONFIG_SET_MSG msg!message length %d is not equal size of Port_Attr_t:%d.\n",datalen,sizeof(SNMP_CONF_INFO_STRU));
                    goto end;
                }
                tSnmpConfigInfo = (SNMP_CONF_INFO_STRU *)(pMsg->data);

                memcpy(&snmpConfInfo, tSnmpConfigInfo, sizeof(SNMP_CONF_INFO_STRU));
                
                tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 0);
                reconfig=1;                
            }
            break;			    
            case IPC_PKT_SNMP_CONFIG_GET_MSG:
            {                  
                  ipc_if_send_ack(ucSrcMo,pMsgOut,0,(char *)&snmpConfInfo,sizeof(SNMP_CONF_INFO_STRU));
                  goto end;
            }
            break;
            default:
                rc = -1;
                strcpy(ErrMsg,"unknown command!\n");
            break;
        }
              ipc_if_send_ack(ucSrcMo,pMsgOut,rc,NULL,0);/*rc��Ϊ����ֵ���ظ��û�*/
#if 0        
        if (rc == 0) {
            ipc_if_send_ack(ucSrcMo,pMsgOut,rc,NULL,0);
        } else {
            log_print(LOG_DEBUG, "message process error.rc:%d,errmsg:%s\n", rc, ErrMsg);
            ipc_if_send_ack(ucSrcMo,pMsgOut,rc,ErrMsg,strlen(ErrMsg)+1);
        }
#endif        

    } 

end:
    ipc_if_free(pMsgOut);
    debuginfo("Pro_Switch_Task end.....\n");
    return 0;

}

void _snmp_reconfig_routine(void)
{
    while(1)
    {
        sleep(1);
        if(reconfig){
            sleep(2);
            _tf_snmp_reconfig();
            reconfig=0;
        }
    }
}

void 
tf_vty_snmp_init(void)
{
    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    ipc_if_init();
    ipc_if_reg_module(MODULE_VTYSH, "VTYSH", process_vtysh_task);

    system("mkdir -p "SNMP_CONF_DIR);

    tf_snmp_conf_update(SNMP_CONF_FULL_NAME, 0);

    tf_vty_snmp_install();
        
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);    
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_getschedparam(&attr, &param);
    
    param.sched_priority = 80;
    pthread_attr_setschedparam(&attr, &param);    
    //pthread_attr_setstacksize(&attr, 1024*1024);

    if(pthread_create(&tid, &attr, (void *)_snmp_reconfig_routine, NULL) != 0)
    {
        printf("Start _snmp_reconfig_routine failed.");
        return ;
    }
}


