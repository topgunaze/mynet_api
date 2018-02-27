/*******************************************************************************
* Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
* 文   件   名: tf_alarm.h
* 作           者: jq.deng
* 版   本   号: V1.0.0
* 生成日期: 2016年9月2日
* 功能描述: 
*******************************************************************************/

#ifndef __TF_ALARM_H__
#define __TF_ALARM_H__

#include "tf_types.h"

#define MAX_EVENT_LOG_TABLE_ENTRY_NUM   256
#define MAX_ALARM_ACTIVE_TABLE_ENTRY_NUM   4096
#define MAX_ALARM_HISTORY_TABLE_ENTRY_NUM   256
#define MAX_ALARM_FILTER_TABLE_ENTRY_NUM   256
#define MAX_ALARM_NUM   (MAX_EVENT_LOG_TABLE_ENTRY_NUM+MAX_ALARM_ACTIVE_TABLE_ENTRY_NUM+MAX_ALARM_HISTORY_TABLE_ENTRY_NUM+MAX_ALARM_FILTER_TABLE_ENTRY_NUM)
#define MAX_ALARM_SEQ_NUM   MAX_ALARM_ACTIVE_TABLE_ENTRY_NUM
#define MAX_EVETN_SEQ_NUM   MAX_EVENT_LOG_TABLE_ENTRY_NUM
#define MAX_ALARM_INSTANCE_LEN   8
#define MAX_ALARM_ADDITIONAL_TEXT_LEN   256


/*alarm severity type definition*/
#define SEVERITYTYPE_CRITICAL     1
#define SEVERITYTYPE_MAJOR         2
#define SEVERITYTYPE_MINOR         3
#define SEVERITYTYPE_WARNING     4
#define SEVERITYTYPE_INFO            5
#define SEVERITYTYPE_CLEAR          6
#define MAX_SEVERITYTYPE_NUM    5    /*SEVERITYTYPE_CLEAR 不计算在内*/


/**************alarm code and severity type definition*****************/

/*board level*/
#define BOARD_PROVISION_FAIL_ALARM   101
#define BOARD_RESET_ALARM   102
#define BOARD_REMOVAL_ALARM   103
#define BOARD_TEMP_ABNORMAL_ALARM   104
#define FAN_FAILED_ALARM   105
#define FAN_REMOVAL_ALARM  106
#define POWER_FAILED_ALARM   107
#define POWER_REMOVAL_ALARM   108
#define FATAL_OLT_HW_ERROR_ALARM   109

#define BOARD_PROVISION_FAIL_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define BOARD_RESET_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define BOARD_REMOVAL_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define BOARD_TEMP_ABNORMAL_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define FAN_FAILED_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define FAN_REMOVAL_SEVERITYTYPE  SEVERITYTYPE_CRITICAL
#define POWER_FAILED_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define POWER_REMOVAL_SEVERITYTYPE   SEVERITYTYPE_CRITICAL
#define FATAL_OLT_HW_ERROR_SEVERITYTYPE   SEVERITYTYPE_MAJOR

/*sni/tf level*/
#define TF_LOOPBACK_ALARM   201
#define OLT_TF_OPTICAL_MODULE_FAULTY_ALARM   202
#define SNI_OPERATION_DISABLED_ALARM   203
#define TF_OPERATION_DISABLED_ALARM   204
#define NUM_OFLINKS_EXCEEDED_ALARM   205
#define TF_LONGEMIT_ALARM   206

#define TF_LOOPBACK_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define OLT_TF_OPTICAL_MODULE_FAULTY_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define SNI_OPERATION_DISABLED_SEVERITYTYPE   SEVERITYTYPE_CRITICAL
#define TF_OPERATION_DISABLED_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define NUM_OFLINKS_EXCEEDED_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define TF_LONGEMIT_SEVERITYTYPE   SEVERITYTYPE_MAJOR


/*onu level*/
#define ONU_CRITICAL_EVENT_ALARM   301
#define ONU_KEY_EXCHANGE_FAILURE_ALARM   302
#define ONU_OAM_TIMEOUT_ALARM   303
#define ONU_MAC_AUTH_FAIL_ALARM   304
#define ONU_US_REVEIVE_SENSITIVITY_LOW_ALARM   305
#define ONU_US_REVEIVE_SENSITIVITY_HIGH_ALARM   306
#define ONU_DS_REVEIVE_SENSITIVITY_LOW_ALARM   307
#define ONU_DS_REVEIVE_SENSITIVITY_HIGH_ALARM   308
#define ONU_POWER_DOWN_ALARM   310
#define DOWN_STREAM_BER_ALARM   311
#define DOWN_STREAM_FER_ALARM   312
#define UP_STREAM_BER_ALARM   313
#define UP_STREAM_FER_ALARM   314
#define STATS_THRESHOLD_UPPER_CROSSED_ALARM   315
#define STATS_THRESHOLD_LOWER_CROSSED_ALARM   316
#define ONU_TEMP_HIGH_ALARM   317
#define ONU_TEMP_LOW_ALARM   318
#define ONU_VCC_HIGH_ALARM   319
#define ONU_VCC_LOW_ALARM   320

#define ONU_CRITICAL_EVENT_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define ONU_KEY_EXCHANGE_FAILURE_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define ONU_OAM_TIMEOUT_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define ONU_MAC_AUTH_FAIL_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define ONU_US_REVEIVE_SENSITIVITY_LOW_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_US_REVEIVE_SENSITIVITY_HIGH_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_DS_REVEIVE_SENSITIVITY_LOW_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_DS_REVEIVE_SENSITIVITY_HIGH_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_POWER_DOWN_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define DOWN_STREAM_BER_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define DOWN_STREAM_FER_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define UP_STREAM_BER_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define UP_STREAM_FER_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define STATS_THRESHOLD_UPPER_CROSSED_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define STATS_THRESHOLD_LOWER_CROSSED_SEVERITYTYPE   SEVERITYTYPE_MAJOR
#define ONU_TEMP_HIGH_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_TEMP_LOW_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_VCC_HIGH_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define ONU_VCC_LOW_SEVERITYTYPE   SEVERITYTYPE_MINOR


/*onu uni level*/
#define UNI_OPERATION_DISABLED_ALARM   401
#define UNI_LOOPBACK_ALARM   402
#define UNI_ETH_AUTO_NEGOTIATION_FAILURE_ALARM   403

#define UNI_OPERATION_DISABLED_SEVERITYTYPE   SEVERITYTYPE_WARNING
#define UNI_LOOPBACK_SEVERITYTYPE   SEVERITYTYPE_MINOR
#define UNI_ETH_AUTO_NEGOTIATION_FAILURE_SEVERITYTYPE   SEVERITYTYPE_WARNING

/**********************event code definition**********************/

/*device level*/
#define DEVICE_RESET_EVENT   10001

/*onu level*/
#define ONU_REGISTERED_EVENT   13001
#define ONU_DEREGISTERED_EVENT   13002

/*onu uni level*/
#define NEW_CBAT_RECOGNIZED_EVENT   14001
#define EXITED_CBAT_UPDATE_EVENT   14002
#define EXITED_CBAT_REMOVE_EVENT   14003



typedef struct TF_ALARM_INFO
{
    unsigned int seqNum;
    unsigned int raisingNum;
    unsigned int correlationId;    
    unsigned int code;

    /*
        The seventh and eighth OCTETS are reserved and shall be set to 
        0 at any time, and shall be ignored by maangement system.        
        For example,        
        0x01 02 03 04 05 06 00 00, represents OLT device 1, slot 2, port 3, 
        ONU logical ID 4, slot 5 UNI port 6 of the connected ONU.
    */
    char instance[MAX_ALARM_INSTANCE_LEN];
    
    unsigned int severity;
    struct tm firstOccurTime;
    struct tm lastOccurTime;
    struct tm clearTime;
    unsigned int repeats;
    char additionalText[MAX_ALARM_ADDITIONAL_TEXT_LEN];      
    char additionalText_len;   
}TF_ALARM_INFO_STRU;



/**********************    构建告警hash 链表所需要的数据结构(start) ******************/

#define NUMBER_OF_ALARM_ENTRIES   MAX_ALARM_NUM
#define ALARM_ENTRIES_HASH_MASK   MAX_SEVERITYTYPE_NUM-1 
#define NUMBER_OF_ALARM_ENTRIES_IN_HASH_TABLE  (ALARM_ENTRIES_HASH_MASK + 1)

typedef struct  ALARM_ENTRY_LINK
{
    struct  ALARM_ENTRY_LINK      *pForwardLink;
    struct  ALARM_ENTRY_LINK      *pBackwardLink;
} ALARM_ENTRY_LINK;

typedef struct
{
    ALARM_ENTRY_LINK             links;
    //ALARM_ENTRY_KEY        key;
    TF_ALARM_INFO_STRU       data;
}ALARM_DATABASE_ENTRY;

/**********************    hash 表数据结构******************/
typedef struct
{
    ALARM_DATABASE_ENTRY   *pForwardLink;
    ALARM_DATABASE_ENTRY   *pBackwardLink;
} ALARM_ENTRY_LOOKUP_TABLE_POINTERS;

typedef struct
{
    ALARM_ENTRY_LOOKUP_TABLE_POINTERS entry[NUMBER_OF_ALARM_ENTRIES_IN_HASH_TABLE];
} ALARM_ENTRY_HASH_TABLE;

typedef struct
{
    ALARM_DATABASE_ENTRY   entry[NUMBER_OF_ALARM_ENTRIES];
} ALARM_ENTRY_DATABASE_TABLE;

/**********************    数据库数据结构******************/
typedef struct
{    
    ALARM_ENTRY_LINK                 freeFdEntriesQueue;

    ALARM_ENTRY_DATABASE_TABLE       *pFdTable;
    ALARM_DATABASE_ENTRY             *pFirstEntryInFdTable;  
    
    ALARM_ENTRY_HASH_TABLE   *pActiveAlarmHashTable;
    ALARM_ENTRY_LOOKUP_TABLE_POINTERS   *pHistoryAlarmTable;
    ALARM_ENTRY_LOOKUP_TABLE_POINTERS   *pFilterAlarmable;    
    ALARM_ENTRY_LOOKUP_TABLE_POINTERS   *pEventLogTable;
} ALARM_ENTRY_LIST;

/**********************    构建告警hash 链表所需要的数据结构(end) ******************/


/**************性能统计门限告警相关数据结构(start)******************/

typedef enum{
    SNMP_InOctets,                              /*0*/
    SNMP_InPkts,                   
    SNMP_InBroadcastPkts,          
    SNMP_InMulticastPkts,          
    SNMP_InPkts64Octets,           
    SNMP_InPkts65to127Octets,        
    SNMP_InPkts128to255Octets,     
    SNMP_InPkts256to511Octets,     
    SNMP_InPkts512to1023Octets,    
    SNMP_InPkts1024to1518Octets, 
    SNMP_InPkts1519to1522Octets,   /*10*/
    SNMP_InUndersizePkts,        
    SNMP_InOversizePkts,         
    SNMP_InFragments,            
    SNMP_InMpcpFrames,           
    SNMP_InMpcpOctets,           
    SNMP_InOAMFrames,            
    SNMP_InOAMOctets,            
    SNMP_InCRCErrorPkts,         
    SNMP_InDropEvents,           
    SNMP_InJabbers,              /*20*/
    SNMP_InCollision,            
    SNMP_OutOctets,              
    SNMP_OutPkts,                
    SNMP_OutBroadcastPkts,       
    SNMP_OutMulticastPkts,       
    SNMP_OutPkts64Octets,        
    SNMP_OutPkts65to127Octets,   
    SNMP_OutPkts128to255Octets,  
    SNMP_OutPkts256to511Octets,  
    SNMP_OutPkts512to1023Octets,      /*30*/
    SNMP_OutPkts1024to1518Octets,
    SNMP_OutPkts1519o1522Octets, 
    SNMP_OutUndersizePkts,       
    SNMP_OutOversizePkts,        
    SNMP_OutFragments,           
    SNMP_OutMpcpFrames,          
    SNMP_OutMpcpOctets,          
    SNMP_OutOAMFrames,           
    SNMP_OutOAMOctets,           
    SNMP_OutCRCErrorPkts,          /*40*/
    SNMP_OutDropEvents,          
    SNMP_OutJabbers,             
    SNMP_OutCollision,
    SNMP_Reserved_44,     /*45-49, 广电没有定义*/
    SNMP_Reserved_45,
    SNMP_Reserved_46,
    SNMP_Reserved_47,
    SNMP_Reserved_48,
    SNMP_Temperature,
    SNMP_Voltage,                    /*50*/
    SNMP_TXPower,
    SNMP_RXPower,
    
    ETF_STATS_THRESHOLD_TYPE_NUM
}ETF_STATS_THRESHOLD_TYPE;

typedef struct STATS_THRESHOLD
{
    UINT8 usedFlag;    /*1:used, 0: no used */
    UINT64 upper;
    UINT64 lower;
    UINT64 counter;
}STATS_THRESHOLD_STRU;

typedef struct 
{
    UINT32 ifindex;    
    UINT8 upperAlarmFlag;   /*1:active, 0:no active*/    
    UINT8 lowerAlarmFlag;   /*1:active, 0:no active*/    
    STATS_THRESHOLD_STRU  threshold[ETF_STATS_THRESHOLD_TYPE_NUM];
}PORT_STATS_THRESHOLD_INFO;

/**************性能统计门限告警相关数据结构(start)******************/


#endif /* __TF_ALARM_H__ */



