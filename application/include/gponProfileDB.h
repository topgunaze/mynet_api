/**************************************************************
* 文件名称:  gtfProfileDB.h
* 作           者:  keith.gong
* 日           期:  2015.11.26
* 文件描述:
* 版           本:  V1.00
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>
        keith.gong          2015.11.26      v1.00              Create
**************************************************************/


#ifndef __GTF_PROFILE_DB_H__
#define __GTF_PROFILE_DB_H__


#include "tf_types.h"
#include "sys_common.h"

#pragma pack(1)


#define INVALID_PROFILE_ID          0xFFFF

#define MAX_TCONT_NUM               5
#define MAX_TCONT_NUM_WITH_DEF      ((MAX_TCONT_NUM)+1)
#define MAX_GEM_NUM                 24

typedef struct S_LIST
{
    struct S_LIST *p_next;
}S_LIST;

//#if 000
/******************************************      ONU Info    **************************************************/
#define CFG_BIT_SET(pOnuInfo, bit)  (pOnuInfo)->cfg_bit |= (bit)

#define ONUCFG_BIT_MIB_AUDIT        0x00000001U
#define ONUCFG_BIT_MIB_RESYNC       0x00000002U
#define ONUCFG_BIT_DBA              0x00000004U
#define ONUCFG_BIT_GEMPORT          0x00000008U
#define ONUCFG_BIT_ALLOCID          0x00000010U
#define ONUCFG_BIT_OMCI_GEMPORT     0x00000020U
#define ONUCFG_BIT_ETH_ATTR         0x00000040U
#define ONUCFG_BIT_VOIP_ATTR        0x00000080U
#define ONUCFG_BIT_POTS_ATTR        0x00000100U
#define ONUCFG_BIT_CATV_ATTR        0x00000200U

#define ONUCFG_BIT_ALL_CFG          0x000003FEU /* 请勿包含ONUCFG_BIT_MIB_AUDIT */

typedef enum
{
    ONU_CFG_STATUS_NA_NR,           /* 未授权未注册00*/
    ONU_CFG_STATUS_NA_R,            /* 未授权，注册01*/
    ONU_CFG_STATUS_A_NR,            /* 授权，未注册10*/
    ONU_CFG_STATUS_A_R,             /* 授权，注册      11*/
}ONU_CFG_STATUS;

typedef struct{
    UINT16                          allocId;
    UINT32                          fix;
    UINT32                          assure;
    UINT32                          max;
}TCONT_INFO_FOR_OMCI_T;

typedef struct{
    UINT16                          tcontId;
    UINT16                          gemPort;
}GEM_INFO_FOR_OMCI_T;

typedef struct
{
    UINT8                           speed_duplex;
    UINT8                           shutdown;
    UINT8                           flow_ctrl;
    UINT16                          def_vlan_tag; /*vid+pri*/
    UINT16                          up_car;
    UINT16                          down_car;
} ONU_ETH_INFO;

#define SIPUSER_NAME_LEN            24
#define SIPUSER_PWD_LEN             24
#define SIPUSER_TELNO_LEN           31

typedef struct
{
    UINT8                           ip_idx;
    UINT8                           mode;
    UINT32                          ip;
    UINT32                          mask;
    UINT32                          gateway;
    UINT32                          primary_dns;
    UINT32                          slave_dns;
    UINT16                          vlan_id;
    UINT8                           priority;
} ONU_IPHOST_INFO;

typedef struct
{
    UINT8                           shutdown;
} ONU_CATV_INFO;

//#endif

/*******************************************   Line Profile   **************************************************/
#define MAX_LINE_PROFILE_NUM        513
#define LINE_PROFILE_NAME_LEN       33
#define VALIDATE_LINE_PROFILE_ID(profile_id) \
    (((profile_id) > 0) && ((profile_id) < MAX_LINE_PROFILE_NUM))

#define VALIDATE_TCONT_ID(tcontId)  ((tcontId) >= 1 && (tcontId) <= MAX_TCONT_NUM)

#define MAX_GEM_MAPPING_NUM_SINGLE_GEMPORT  8

#define DEFAULT_8021P_PRIORITY      0
#define INVALID_8021P_PRIORITY      0xFF
#define DEFAULT_VLAN_ID             1
#define INVALID_VLAN_ID             0xFFFF
#define MAX_VID_PER_GEMPORT         12

#define INVALID_ALLOC_ID            0xFFFF
#define INVALID_GEMPORT_ID          0



typedef enum{
    GEM_MAPPING_MODE_VLAN,
    GEM_MAPPING_MODE_PRIORITY,
    GEM_MAPPING_MODE_VLAN_PRIORITY,
}GEM_MAPPING_MODE;

typedef struct{
    UINT8                           MappingUsedOrNot;
    UINT8                           Priority;
    UINT32                          Vlan;
}GEM_MAPPING_INFO;

typedef struct
{
    UINT8                           tcontId;
    UINT16                          up_car;
    UINT16                          down_car;
    UINT8                           GemMappingSum;
    GEM_MAPPING_INFO                mappingRule[MAX_GEM_MAPPING_NUM_SINGLE_GEMPORT];
}LINE_PROFILE_GEM_INFO;

typedef struct
{
    UINT16      line_profile_id;
    UINT16      dba_profile_id;
    
}LINE_PROFILE_TCONT_INFO;

typedef struct ONU_INFO_ENTRY
{
    struct ONU_INFO_ENTRY           *p_next;
    //ONU_INFO                        *pOnuInfo;
}ONU_INFO_ENTRY;

typedef enum
{
    LINE_PROFILE_QOS_MODE_PQ,
    LINE_PROFILE_QOS_MODE_GEM_CAR,
    LINE_PROFILE_QOS_MODE_FLOW_CAR,
}LINE_PROFILE_QOS_MODE;


typedef struct{
    UINT8                           GemportUsedOrNot;
    UINT8                           tcontId;
    UINT16                          up_car;
    UINT16                          down_car;

    UINT8                           GemMappingSum;
    GEM_MAPPING_INFO                mappingRule[MAX_GEM_MAPPING_NUM_SINGLE_GEMPORT];
}GEM_INFO_DATA_T;

typedef struct{
    UINT8                           tcont_is_used;
    UINT16                          dba_profile_id;

    UINT8                           gemport_bitmap[(MAX_GEM_NUM)/8];
    UINT8                           gemport_sum;
}TCONT_INFO_DATA_T;

typedef struct{
    UINT8                           fecUpstream;
    UINT8                           omccEncrypt;
    UINT8                           qosMode;

    UINT8                           gemMappingMode;
    GEM_INFO_DATA_T                 gem_info[MAX_GEM_NUM];

    TCONT_INFO_DATA_T               tcont_info[MAX_TCONT_NUM_WITH_DEF];
}LINE_DATA_T;

typedef struct LINE_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[LINE_PROFILE_NAME_LEN];

    LINE_DATA_T                     data;

    UINT16                          bind_onu_counter;
}LINE_PROFILE_INFO;

/*******************************************   Srv Profile   **************************************************/
#define MAX_SRV_PROFILE_NUM         513
#define SRV_PROFILE_NAME_LEN        33
#define VALIDATE_SRV_PROFILE_ID(profile_id) \
    (((profile_id) > 0) && ((profile_id) < MAX_SRV_PROFILE_NUM))

//#define SYS_MAX_ONU_PORT_NUM        8       /* 系统单ONU 的单卡支持的最大端口数*/
//#define SYS_MAX_ONU_POTS_NUM        8       /* 系统单ONU 的单卡支持的最大POTS 端口数*/
//#define SYS_MAX_ONU_CATV_NUM        8       /* 系统单ONU 的单卡支持的最大CATV 端口数*/


/*--------------   VLAN  --------------*/
#define ONUCFG_ETH_MAX_VLAN_ENTRY   9
typedef enum
{
    ONU_ETH_PORT_VLAN_TP = 0,
    ONU_ETH_PORT_VLAN_NTP,
}ONU_ETH_VLAN_MODE_DTE;


#define PRIORITY_COPY_FROM_INNER 8
#define PRIORITY_COPY_FROM_OUTER 9
#define PRIORITY_DSCP_MAP        10

typedef struct
{
    UINT8                           qinqFlag;
    UINT16                          sVlan;
    UINT8                           sPri;
    UINT16                          cVlan;
    UINT8                           cPri;
}ONU_ETH_VLAN_ENTRY;

typedef struct
{
    ONU_ETH_VLAN_MODE_DTE           vlanMode;
    ONU_ETH_VLAN_ENTRY              entry[ONUCFG_ETH_MAX_VLAN_ENTRY];
} SRV_PROFILE_ETH_VLAN_INFO;


/*--------------   ONU  ETH  PORT   --------------*/
typedef struct
{
    SRV_PROFILE_ETH_VLAN_INFO       vlanParam;               /* 端口VLAN属性*/
} SRV_PROFILE_ETH_INFO;

typedef enum
{
    ONU_MCAST_MODE_IGMP_SNOOPING,
    ONU_MCAST_MODE_OLT_CONTROL,
}ONU_MCAST_MODE;

typedef enum
{
    ONU_MCAST_IGMP_FWD_MODE_DEF,
    ONU_MCAST_IGMP_FWD_MODE_TR,
    ONU_MCAST_IGMP_FWD_MODE_TL,
}ONU_MCAST_IGMP_FWD_MODE;

typedef enum
{
    ONU_MCAST_MC_FWD_MODE_UNTAG,
    ONU_MCAST_MC_FWD_MODE_TAG_TP,
    ONU_MCAST_MC_FWD_MODE_TAG_TL,
}ONU_MCAST_MC_FWD_MODE;

/*--------------   ONU   MCAST   --------------*/
typedef struct
{
    UINT8                           mode;           /* ONU_MCAST_MODE */
    UINT8                           upIgmpFwdMode;    /* ONU_MCAST_IGMP_FWD_MODE */
    UINT16                          upIgmpTCI;
    UINT8                           dnMcMode;
    UINT16                          dnMcTCI;
}ONU_MCAST_T;

/*--------------   ONU   PORT NUM --------------*/
typedef struct
{
    UINT8                           catvPortNum;
    UINT8                           ethPortNum;
    UINT8                           mocaPortNum;
    UINT8                           potsPortNum;
    UINT8                           catvPortNumAdaptive;
    UINT8                           ethPortNumAdaptive;
    UINT8                           mocaPortNumAdaptive;
    UINT8                           potsPortNumAdaptive;
}ONT_PORTS_NUM;

typedef struct
{
    ONT_PORTS_NUM                   portsNum;

    UINT16                          macAgeTime;
    UINT8                           macLearingInd;
    ONU_MCAST_T                     onuMcast;           /*组播:包含组播模式、组播快速离开*/

    SRV_PROFILE_ETH_INFO            onu_eth_attr[SYS_MAX_ONU_PORT_NUM]; /* 以太口属性表指针*/
}SRV_DATA_T;

typedef struct SRV_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[SRV_PROFILE_NAME_LEN];

    SRV_DATA_T                      data;

    UINT16                          bind_onu_counter;
}SRV_PROFILE_INFO;

/*******************************************   DBA Profile   **************************************************/
#define MAX_DBA_PROFILE_NUM         129
#define DBA_PROFILE_NAME_LEN        17
#define VALIDATE_DBA_PROFILE_ID(profile_id)  \
    ((profile_id) < MAX_DBA_PROFILE_NUM)
#define VALIDATE_USER_DBA_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_DBA_PROFILE_NUM))

#define MAX_TF_BW                  1060864 //(1036*1024) (136832000 - 33000 * 128) /128000  = 1036
#define MAX_TF_BW_STR              "1060864" //(1036*1024)
#define DBA_DEFAULT_BW              128
#define DEF_OMCC_FIX_BW             32

typedef enum
{
    DBA_TYPE_FIX = 1,
    DBA_TYPE_ASSURE,
    DBA_TYPE_ASSURE_AND_MAX,
    DBA_TYPE_MAX,
    DBA_TYPE_FIX_AND_ASSURE_AND_MAX,

    DBA_TYPE_INVALID,
}DBA_TYPE_DTE;

typedef struct
{
    UINT8                           type;
    UINT32                          fix;
    UINT32                          assure;
    UINT32                          max;
}DBA_DATA_T;

#define LINE_PROFILE_TCONT_INFO_TO_TCONT_ENTRY(pTcontInfo) \
    (LINE_PROFILE_TCONT_ENTRY*)((UINT8*)pTcontInfo - sizeof(struct LINE_PROFILE_TCONT_ENTRY*))

typedef struct LINE_PROFILE_TCONT_ENTRY
{
    struct LINE_PROFILE_TCONT_ENTRY *p_next;
    LINE_PROFILE_TCONT_INFO          tcont_info;
}LINE_PROFILE_TCONT_ENTRY;

typedef struct DBA_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[DBA_PROFILE_NAME_LEN];
    DBA_DATA_T                      dba;

    UINT16                          bind_tcont_counter;
    //LINE_PROFILE_TCONT_ENTRY        *p_tcont_list;
}DBA_PROFILE_INFO;


/*****************************************   sip agent Profile   ************************************************/
#define MAX_SIPAGENT_PROFILE_NUM        17
#define SIPAGENT_PROFILE_NAME_LEN       17
#define VALIDATE_SIPAGENT_PROFILE_ID(profile_id)  \
    ((profile_id) < MAX_SIPAGENT_PROFILE_NUM)
#define VALIDATE_USER_SIPAGENT_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_SIPAGENT_PROFILE_NUM))

#define GTF_SIPAGENT_URI_LEN           63
#define GTF_SIPAGENT_AUTH_REALM_LEN    24

#define SIGNAL_TRANSFER_MODE_UDP        0
#define SIGNAL_TRANSFER_MODE_TCP        1

typedef struct
{
    char                            proxyServerUri[GTF_SIPAGENT_URI_LEN];

    UINT8                           rtpDscp;
    UINT16                          rtpMinPort;
    UINT16                          rtpMaxPort;

    UINT8                           signalDscp;
    UINT16                          signalPort;
    UINT8                           signalTransfermode;

    UINT32                          registrationExpiration;
    UINT32                          registrationReregHeadStartTime;
    char                            registrationServerUri[GTF_SIPAGENT_URI_LEN];

    char                            voiceMailServerUri[GTF_SIPAGENT_URI_LEN];
    UINT32                          voiceMailSubscriptionExpiration;

    char                            confFactory[GTF_SIPAGENT_URI_LEN];
    char                            bridgedLineAgent[GTF_SIPAGENT_URI_LEN];
    char                            authRealm[GTF_SIPAGENT_AUTH_REALM_LEN];
}SIPAGENT_DATA_T;


typedef struct ONU_POTS_ENTRY
{
    struct ONU_POTS_ENTRY           *p_next;
    UINT8                           port_id;
    UINT16                          onu_id;
    UINT32                          port_bitmap;
}ONU_POTS_ENTRY;

typedef struct SIPAGENT_PROFILE_INFO
{
    UINT16                         profile_id;
    char                           profile_name[SIPAGENT_PROFILE_NAME_LEN];

    SIPAGENT_DATA_T                data;

    UINT16                         bind_pots_counter;
    ONU_POTS_ENTRY                 *p_onu_list;//业务面使用
}SIPAGENT_PROFILE_INFO;


/*****************************************   pots Profile   ************************************************/
#define MAX_POTS_PROFILE_NUM        17
#define POTS_PROFILE_NAME_LEN       17
#define VALIDATE_POTS_PROFILE_ID(profile_id)  \
        ((profile_id) < MAX_POTS_PROFILE_NUM)
#define VALIDATE_USER_POTS_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_POTS_PROFILE_NUM))

typedef enum
{
    POTS_IMPEDANCE_600OHMS = 0,
    POTS_IMPEDANCE_900OHMS,
    POTS_IMPEDANCE_150NF_750OHM_270OHM,
    POTS_IMPEDANCE_115NF_820OHM_220OHM,
    POTS_IMPEDANCE_230NF_1050OHM_320OHM,
    POTS_IMPEDANCE_MAX,
}POTS_IMPEDANCE_DTE;

#define POTS_GAIN_MIN               (-12)
#define POTS_GAIN_MAX               (6)

typedef enum
{
    SIGNALLING_CODE_LOOP_START = 1,
    SIGNALLING_CODE_GROUND_START,
    SIGNALLING_CODE_LOOP_REVERSE_BATTERY,
    SIGNALLING_CODE_COIN_FIRST,
    SIGNALLING_CODE_DIAL_TONE_FIRST,
    SIGNALLING_CODE_MULTI_PARTY,
}VOICE_CTP_SIGNALLING_CODE_DTE;

typedef struct
{
    UINT8                           impedance;
    UINT8                           signallingCode;
    UINT8                           rxGain;
    UINT8                           txGain;
}POTS_DATA_T;

typedef struct POTS_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[POTS_PROFILE_NAME_LEN];

    POTS_DATA_T                     data;

    UINT16                          bind_pots_counter;
    ONU_POTS_ENTRY                  *p_onu_list;//业务面使用
}POTS_PROFILE_INFO;


/*****************************************   digitmap Profile   ************************************************/
#define MAX_DIGITMAP_PROFILE_NUM        17
#define DIGITMAP_PROFILE_NAME_LEN       17
#define VALIDATE_DIGITMAP_PROFILE_ID(profile_id)  \
        ((profile_id) < MAX_DIGITMAP_PROFILE_NUM)
#define VALIDATE_USER_DIGITMAP_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_DIGITMAP_PROFILE_NUM))

#define DIAL_PLAN_TOKEN_NUM         10
#define DIAL_PLAN_TOKEN_NUM_STR     "10" /* DIAL_PLAN_TOKEN_NUM */
#define DIAL_PLAN_TOKEN_LEN         28

typedef enum
{
    DIGITI_MAP_FORMAT_NOT_DEFINED,
    DIGITI_MAP_FORMAT_H248,
    DIGITI_MAP_FORMAT_NCS,
    DIGITI_MAP_FORMAT_VENDOR_SPECIFIC,
}DIGITI_MAP_FORMAT_DTE;

typedef struct
{
    UINT8                           valid;
    char                            token[DIAL_PLAN_TOKEN_LEN];
}DIAL_PLAN_ENTRY;

typedef struct
{
    UINT16                          criticalDialTime; /*1~65535 */
    UINT8                           digitmapFormat;
    UINT16                          partialDialTime; /*1~65535 */
    DIAL_PLAN_ENTRY                 dialPlanTable[DIAL_PLAN_TOKEN_NUM];
}DIGITMAP_DATA_T;

typedef struct DIGITMAP_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[DIGITMAP_PROFILE_NAME_LEN];

    DIGITMAP_DATA_T                 data;

    UINT16                          bind_pots_counter;
    ONU_POTS_ENTRY                  *p_onu_list;//业务面使用
}DIGITMAP_PROFILE_INFO;


/*****************************************   sipright Profile   ************************************************/
#define MAX_SIPRIGHT_PROFILE_NUM        17
#define SIPRIGHT_PROFILE_NAME_LEN       17
#define VALIDATE_SIPRIGHT_PROFILE_ID(profile_id)  \
        ((profile_id) < MAX_SIPRIGHT_PROFILE_NUM)
#define VALIDATE_USER_SIPRIGHT_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_SIPRIGHT_PROFILE_NUM))

#define SIPRIGHT_BIT_THREE_PARTY    0x0001U     /*1*/
#define SIPRIGHT_BIT_CALL_TRANSFER  0x0002U     /*0*/
#define SIPRIGHT_BIT_CALL_HOLD      0x0004U     /*0*/
#define SIPRIGHT_BIT_CALL_PARK      0x0008U     /*0*/
#define SIPRIGHT_BIT_DO_NOT_DISTURB 0x0010U     /*1*/
#define SIPRIGHT_BIT_CONF           0x0080U     /*1*/

#define SIPRIGHT_BIT_HOTLINE        0x0001U     /*0*/
#define SIPRIGHT_BIT_HOTLINE_DELAY  0x0002U     /*0*/


#define HOTLINE_NUM_LEN             32

typedef struct
{
    UINT8                           callWaiting;
    UINT8                           callProcess;
    UINT8                           callPresentation;
    UINT8                           hotline;
    char                            hotlineNum[HOTLINE_NUM_LEN];
}SIPRIGHT_DATA_T;

typedef struct SIPRIGHT_PROFILE_INFO
{
    UINT16                          profile_id;
    char                            profile_name[SIPRIGHT_PROFILE_NAME_LEN];

    SIPRIGHT_DATA_T                 data;

    UINT16                          bind_pots_counter;
    ONU_POTS_ENTRY                  *p_onu_list;//业务面使用
}SIPRIGHT_PROFILE_INFO;


/*****************************************   traffic Profile   ***************************************************/
#define MAX_TRAFFIC_PROFILE_NUM         257
#define TRAFFIC_PROFILE_NAME_LEN        17
#define VALIDATE_TRAFFIC_PROFILE_ID(profile_id)  \
    ((profile_id) < MAX_TRAFFIC_PROFILE_NUM)
#define VALIDATE_USER_TRAFFIC_PROFILE_ID(profile_id)  \
        (((profile_id) > 0) && ((profile_id) < MAX_TRAFFIC_PROFILE_NUM))

typedef struct
{
    UINT32                          cir;
    UINT32                          pir;
    UINT32                          cbs;
    UINT32                          pbs;
}TRAFFIC_DATA_T;

typedef struct TRAFFIC_PROFILE_INFO
{
    UINT16                         profile_id;
    char                           profile_name[SIPAGENT_PROFILE_NAME_LEN];

    TRAFFIC_DATA_T                 data;

    UINT16                         bind_counter;
}TRAFFIC_PROFILE_INFO;

typedef struct
{
    UINT8                           shutdown;

    UINT8                           valid;
    char                            name[SIPUSER_NAME_LEN+1];
    char                            password[SIPUSER_PWD_LEN+1];
    char                            telno[SIPUSER_TELNO_LEN+1];
    UINT8                           ip_idx;

    //模板配置
    UINT16                          pots_profile_id;
    UINT16                          sipagent_profile_id;
    UINT16                          sipright_profile_id;
    UINT16                          digitmap_profile_id;

    //离散配置
    POTS_DATA_T                     pots_data;
    SIPAGENT_DATA_T                 sipagent_data;
    SIPRIGHT_DATA_T                 sipright_data;
    DIGITMAP_DATA_T                 digitmap_data;
    
} ONU_POTS_INFO;

typedef struct
{
    UINT8                           status;
    UINT8                           percent;
}
ONU_LOAD_STATE;

//单onu离散配置
typedef struct{
    UINT8                           gemport_is_use;
    UINT8                           tcont_id;
    TRAFFIC_DATA_T                  up_car_info;
    TRAFFIC_DATA_T                  down_car_info;
    
    UINT8                           gem_mapping_sum;
    GEM_MAPPING_INFO                mapping_rule[MAX_GEM_MAPPING_NUM_SINGLE_GEMPORT];
}GEM_INFO_FOR_SINGLE_T;

typedef struct{
    UINT8                           tcont_is_used;
    DBA_DATA_T                      dba_info;

    UINT8                           gemport_bitmap[(MAX_GEM_NUM)/8];
    UINT8                           gemport_sum;
}TCONT_INFO_FOR_SINGLE_T;

typedef struct{
    UINT8                           fec_us_stream;
    UINT8                           omcc_encrypt;
    UINT8                           qos_mode;

    UINT8                           gem_mapping_mode;
    GEM_INFO_FOR_SINGLE_T           gem_info[MAX_GEM_NUM];

    TCONT_INFO_FOR_SINGLE_T         tcont_info[MAX_TCONT_NUM_WITH_DEF];
}LINE_DATA_FOR_SINGLE_T;

typedef struct
{
    /* 状态标识*/
    UINT8                           onu_state;       /* ONU_CFG_STATUS */
    UINT8                           onu_busy_flag;
    UINT8                           mib_sync_flag;    /* MIB 同步标识, 1: 同步，0:未同步*/
    UINT32                          cfg_bit;
    UINT32                          failed_bit;

    UINT32                          phy_uint; //请注意phy_uint可能失效

    /* 模板信息*/
    UINT16                          lineprofile_id;
    UINT16                          srvprofile_id;
    
    /*离散配置信息*/
    LINE_DATA_FOR_SINGLE_T          line_data;
    SRV_DATA_T                      srv_data;

    /* 物理资源信息*/
    TCONT_INFO_FOR_OMCI_T           tcont_info[MAX_TCONT_NUM_WITH_DEF];
    GEM_INFO_FOR_OMCI_T             gem_info[MAX_GEM_NUM];
    GEM_INFO_FOR_OMCI_T             def_gem_info;

    /* 以太网端口信息*/
    ONU_ETH_INFO                    onu_eth_attr[SYS_MAX_ONU_PORT_NUM];

    /* POTS 端口信息*/
    ONU_POTS_INFO                   onu_pots_attr[SYS_MAX_ONU_POTS_NUM];

    /* iphost 端口信息*/
    ONU_IPHOST_INFO                 onu_ip_host_attr[SYS_MAX_ONU_IPHOST_NUM];

    /* CATV 端口信息*/
    ONU_CATV_INFO                   onu_catv_attr[SYS_MAX_ONU_CATV_NUM];

    ONU_LOAD_STATE                  load_state;
    
}ONU_INFO;

#pragma pack()


#endif /* __GTF_PROFILE_DB_H__ */

