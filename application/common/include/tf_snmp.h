/*******************************************************************************
* Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
* 文   件   名: \tf_snmp.h
* 作           者: jq.deng
* 版   本   号: V1.0.0
* 生成日期: 2016年8月31日
* 功能描述: 
*******************************************************************************/

#ifndef __TF_SNMP_H__
#define __TF_SNMP_H__

/*module info*/
#define SNMP_CONF_DIR               "/etc/snmp/"
#define SNMP_PROGRAM_NAME           "snmpd"
#define SNMP_CONF_FULL_NAME         SNMP_CONF_DIR""SNMP_PROGRAM_NAME".conf"

/*sys info*/
#define SNMP_DEF_SYS_DESC           "description"
#define SNMP_SYS_DESC_LEN           100
#define SNMP_SYS_DESC_LEN_STR       "100"
#define SNMP_DEF_SYS_NAME           "name"
#define SNMP_SYS_NAME_LEN           100
#define SNMP_SYS_NAME_LEN_STR       "100"
#define SNMP_DEF_CONTACT            "contact"
#define SNMP_CONTACT_LEN            100
#define SNMP_CONTACT_LEN_STR        "100"
#define SNMP_DEF_LOCATION           "location"
#define SNMP_LOCATION_LEN           100
#define SNMP_LOCATION_LEN_STR       "100"
#define SNMP_DEF_ENABLE             0

/* user info */
#define MAX_TRAP                          4
#define MAX_COMMUNITY               4
#define MAX_USM_USER                10
#define MAX_ACCESS                  10
#define SNMP_TRAP_HOST_NAME_LEN         32
#define SNMP_TRAP_HOST_NAME_LEN_STR      "32"
#define SNMP_COMMUNITY_LEN          32
#define SNMP_COMMUNITY_LEN_STR      "32"
#define SNMP_USM_USER_NAME_LEN      32
#define SNMP_USM_USER_NAME_LEN_STR  "32"
#define SNMP_USER_GROUP_LEN         32
#define SNMP_USER_GROUP_LEN_STR     "32"
#define SNMP_USM_KEY_MIN_LEN        8
#define SNMP_USM_KEY_MIN_LEN_STR    "8"
#define SNMP_USM_AUTH_KEY_LEN       64
#define SNMP_USM_AUTH_KEY_LEN_STR   "64"
#define SNMP_USM_PRIV_KEY_LEN       64
#define SNMP_USM_PRIV_KEY_LEN_STR   "64"
#define SNMP_SEC_LEVEL_LEN          32
#define SNMP_SER_LEVEL_LEN_STR      "32"
#define SNMP_SEC_LEVEL_COMMON_LEN   4

/* view */
#define SNMP_VIEW_NAME_LEN          32
#define SNMP_VIEW_NAME_LEN_STR      "32"

/* vty desc */
#define DESC_SNMP_MODULE            "<Group> snmp-agent command group\n"
#define DESC_SNMP_SYS_INFO_NODE     "System information of the node\n"
#define DESC_SNMP_COMMUNITY_NODE    "Community for the access of SNMPv1 and SNMPv2c\n"
#define DESC_SNMP_USM_USER_NODE     "User for access to SNMP entity based on USM\n"
#define DESC_SNMP_USM_USER_V3_NODE  "SNMPv3 security model\n"
#define DESC_SNMP_GROUP_NODE        "SNMP group based on USM\n"
#define DESC_SNMP_GROUP_V3_NODE     "SNMPv3 vacm group\n"
#define DESC_SNMP_MIB_VIEW_NODE     "SNMP MIB view information\n"
#define DESC_SNMP_STATE_NODE        "SNMP operation state.\n"
#define DESC_SNMP_CONFIG_NODE       "SNMP configuration.\n"

#define SNMP_TRAP_DEF_PORT 162
    
/*sys info*/
typedef struct SNMP_SYS_INFO
{
    char sysContact[SNMP_CONTACT_LEN+1];
    char sysLocation[SNMP_LOCATION_LEN+1];
    char sysName[SNMP_SYS_NAME_LEN+1];
    char sysDesc[SNMP_SYS_DESC_LEN+1];
    char enable;
}SNMP_SYS_INFO_STRU;

/*v1,v2c*/
typedef enum SNMP_RIGHT
{
    SNMP_RIGHT_INVALID,
    SNMP_RIGHT_RO,
    SNMP_RIGHT_RW,
}SNMP_RIGHT_ENUM;

typedef struct SNMP_COMMUNITY_INFO
{
    SNMP_RIGHT_ENUM type;
    char community[SNMP_COMMUNITY_LEN+1];
}SNMP_COMMUNITY_INFO_STRU;

/*v3*/
typedef struct SNMP_USM_USER_INFO
{
    char name[SNMP_USM_USER_NAME_LEN+1];
    char group[SNMP_USER_GROUP_LEN+1];
    char authpass[SNMP_USM_AUTH_KEY_LEN+1];
    char privpass[SNMP_USM_PRIV_KEY_LEN+1];
}SNMP_USM_USER_INFO_STRU;

typedef struct ACCESS
{
    char group[SNMP_USER_GROUP_LEN+1];
    char secLevel[SNMP_SEC_LEVEL_LEN+1];
    char readView[SNMP_VIEW_NAME_LEN+1];
    char writeView[SNMP_VIEW_NAME_LEN+1];
    char notifyView[SNMP_VIEW_NAME_LEN+1];
}ACCESS_STRU;

typedef struct SNMP_TRAP_INFO
{
    unsigned int ipAddress;
    unsigned int port; 
    char hostName[SNMP_TRAP_HOST_NAME_LEN+1];
    char community[SNMP_COMMUNITY_LEN+1];
}SNMP_TRAP_INFO_STRU;

/*snmp conf*/
typedef struct SNMP_CONF_INFO
{
    SNMP_SYS_INFO_STRU  sysInfo;
    SNMP_COMMUNITY_INFO_STRU snmpCommunity[MAX_COMMUNITY];
    SNMP_USM_USER_INFO_STRU usmUserInfo[MAX_USM_USER];
    ACCESS_STRU access[MAX_ACCESS];
    SNMP_TRAP_INFO_STRU trapInfo[MAX_TRAP];
}SNMP_CONF_INFO_STRU;

#endif /* __TF_SNMP_H__ */

