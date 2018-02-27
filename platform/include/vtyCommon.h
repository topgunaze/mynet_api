/**************************************************************
 * 文件名称:  vtyCommon.h
 * 作           者:  keith.gong
 * 日           期:  2015.10.29
 * 文件描述:  VTY通用定义
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef _VTY_COMMON_H_
#define _VTY_COMMON_H_

#ifndef FD1616S
#define FD1616S
#endif

#ifdef FD1216S
#define TF_CMD_STR      "<1-16>"
#define GE_F_CMD_STR      "<1-4>" 
/*表示fibei端口*/
#define GE_CMD_STR      "<1-8>"
#define XGE_CMD_STR      "<1-2>"
#define SA_CMD_STR      "<1-8>"
#define LACP_CMD_STR      "<9-16>"
#define DESC_TF_PORT_LIST     "Port list. <L><1-16>(e.g.:1,3-5,8)\n"
#define DESC_TF_PORT_ID       "Port ID. <U><1-16>\n"
#define DESC_GE_PORT_LIST       "Port list. <L><1-8>(e.g.:1,3-5,8)\n"
#define DESC_GE_PORT_ID         "Port ID. <U><1-8>\n"
#define DESC_PORT_LIST          "Port list. <U><1-8>(e.g.:1,3-5,8)\n"
#define DESC_SA_PORT_LIST       "Port list. <L><1-8>(e.g.:1,1-2,8)\n"
#define DESC_SA_PORT_ID         "Port ID. <U><1-8>\n"

#elif  defined(FD1104Y)
#define TF_CMD_STR      "<1-4>"
#define GE_CMD_STR      "<1-4>"
#define DESC_TF_PORT_LIST     "Port list. <L><1-4>(e.g.:1,3-5,8)\n"
#define DESC_TF_PORT_ID       "Port ID. <U><1-4>\n"
#define DESC_GE_PORT_LIST       "Port list. <L><1-4>(e.g.:1,3-5,8)\n"
#define DESC_GE_PORT_ID         "Port ID. <U><1-4>\n"
#define DESC_PORT_LIST          "Port list. <U><1-4>(e.g.:1,3-5,8)\n"
#define DESC_SA_PORT_LIST       "Port list. <L><1-4>(e.g.:1,1-2,8)\n"
#define DESC_SA_PORT_ID         "Port ID. <U><1-4>\n"

#elif defined(FD1616S)
#define TF_CMD_STR      "<1-16>"
#define GE_F_CMD_STR      "<1-4>" 
/*表示fibei端口*/
#define GE_CMD_STR      "<1-4>"
#define XGE_CMD_STR      "<1-2>"
#define SA_CMD_STR      "<1-8>"
#define LACP_CMD_STR      "<9-16>"
#define DESC_TF_PORT_LIST     "Port list. <L><1-16>(e.g.:1,3-5,8)\n"
#define DESC_TF_PORT_ID       "Port ID. <U><1-16>\n"
#define DESC_GE_PORT_LIST       "Port list. <L><1-4>(e.g.:1,3-4)\n"
#define DESC_GE_PORT_ID         "Port ID. <U><1-4>\n"
#define DESC_PORT_LIST          "Port list. <U><1-8>(e.g.:1,3-5,8)\n"
#define DESC_SA_PORT_LIST       "Port list. <L><1-8>(e.g.:1,1-2,8)\n"
#define DESC_SA_PORT_ID         "Port ID. <U><1-8>\n"

#define CONFIG_INTERFACE_GTF    "interface gtf"
#define DESC_SA_LACP_PORT_LIST  "Port list. <L>(e.g.:1, 3-5, 8), manual""<1-8>"",  lacp""<9-16>""\n"
#define PROTOCOL_VLAN_CMD_STR   "<1-16>"
#define CMD_ACL_INBOUND_STR  "inbound"
#define CMD_COS_QUEUE_WRR_STR   "<0-100> <0-100> <0-100> <0-100>"
#define CMD_COS_QUEUE_MAP_STR "{cos0 <0-7>|cos1 <0-7>|cos2 <0-7>|cos3 <0-7>|cos4 <0-7>|cos5 <0-7>|cos6 <0-7>|cos7 <0-7>}"
#define CMD_COS_QUEUE_WRED_STR "{queue0 <0-7>|queue1 <0-7>|queue2 <0-7>|queue3 <0-7>}"
#define CMD_NO_COS_QUEUE_WRED_STR "{queue0|queue1|queue2|queue3}"
#define CMD_EARLY_DROP_STR "{cos0 <0-100>|cos1 <0-100>|cos2 <0-100>|cos3 <0-100>}"
#define CMD_DEST_PORT_ID_STR    "(ge ""<1-4>"")"
#define CMD_PORT_LIST_STR       "(gtf PORT-LIST|ge PORT-LIST|xge PORT-LIST)"
#define CMD_PORT_ID_STR         "(gtf ""<1-16>""|ge ""<1-4>""|xge ""<1-2>"")"

#else 
#define TF_CMD_STR      "<1-8>"
#define GE_CMD_STR      "<1-8>"
#define DESC_TF_PORT_LIST     "Port list. <L><1-8>(e.g.:1,3-5,8)\n"
#define DESC_TF_PORT_ID       "Port ID. <U><1-8>\n"
#define DESC_GE_PORT_LIST       "Port list. <L><1-8>(e.g.:1,3-5,8)\n"
#define DESC_GE_PORT_ID         "Port ID. <U><1-8>\n"
#define DESC_XGE_PORT_ID        "Port ID. <U><1-2>\n"
#define DESC_PORT_LIST          "Port list. <U><1-8>(e.g.:1,3-5,8)\n"
#define DESC_SA_PORT_LIST       "Port list. <L><1-8>(e.g.:1,1-2,8)\n"
#define DESC_SA_PORT_ID         "Port ID. <U><1-8>\n"

#endif

#define XGE_SUPPORT

#define VLAN_CMD_STR      "<1-4094>"

#define VTY_CURRENT_SLOT_ID (vty->user.env.slot_id >> 16)

#define VTY_CURRENT_TF_ID (vty->user.env.tfId & 0x0000ffff)

#define VTY_SLOT_ID2ENV(env_id,slot_id) \
    do{\
        (env_id) = ((slot_id) << 16) | ((env_id) & 0x0000ffff);\
    }while(0)
    
#define VTY_TF_ID2ENV(env_id,port_id) \
    do{\
        (env_id) = ((env_id) & 0xffff0000) | (port_id);\
    }while(0)


#define DESC_XGE_PORT_LIST      "Port list. <L><1-2>(e.g.:1,1-2,2)\n"
#define DESC_XGE_PORT_ID        "Port ID. <U><1-2>\n"

#define DESC_NO                 "Negate a command or set its defaults\n"
#define DESC_SHOW               "Show information\n"
#define DESC_EXIT               "Exit from current mode and enter prior mode\n"
#define DESC_END                "End current mode and change to view mode\n"
#define DESC_DEBUGGING          "<Group> debugging command group\n"

#define DESC_MAC_ADDR           "MAC address. <M><XX:XX:XX:XX:XX:XX>\n"
#define DESC_VLAN_LIST          "VLAN list. <L><1-4094>(e.g.:1,11-27,4094)\n"
#define DESC_VLAN_ID            "VLAN ID. <U><1-4094>\n"
#define DESC_VLAN_PRI           "VLAN tag priority. <U><0-7>\n"
//#define DESC_TF_PORT_LIST     "Port list. <L><1-16>(e.g.:1,3-5,8)\n"
#define DESC_TF_PORT          "Gtf port.\n"
#define DESC_TF_PORT_ID       "Port ID. <U><1-16>\n"

//#define DESC_GE_PORT_LIST       "Port list. <L><1-8>(e.g.:1,3-5,8)\n"
#define DESC_GE_PORT            "Ge port.\n"
//#define DESC_GE_PORT_ID         "Port ID. <U><1-8>\n"
#define DESC_XGE_PORT_LIST      "Port list. <L><1-2>(e.g.:1,1-2,2)\n"
#define DESC_XGE_PORT           "XGe port.\n"
#define DESC_XGE_PORT_ID        "Port ID. <U><1-2>\n"
#define DESC_PORT_LIST          "Port list. <U><1-8>(e.g.:1,3-5,8)\n"
#define DESC_SA_PORT_LIST       "Port list. <L><1-8>(e.g.:1,1-2,8)\n"
#define DESC_SA_PORT_ID         "Port ID. <U><1-8>\n"
#define DESC_GTF_ONT_ID        "ONT ID. <U><1-128>\n"
#define DESC_GTF_ONT_ID_LIST   "ONT ID list. <L><1-128>(e.g.:1-17,128)\n"
#define DESC_GTF_ONT_ALL       "All ONTS in the port\n"


#define DESC_LOID_VALUE         "Logic ONU identifier. <S><Length 1-24>\n"
#define DESC_PASSWORD_VALUE     "Password. <S><Length 1-12>\n"

#define DESC_GTF_PORT_ALL      "All PORTS in the gtf\n"

#define DESC_GTF_ONT_SN        "ONT serial number. <S><Length 12,13,16>(XXXXXXXXXXXX, XXXX-XXXXXXXX, XXXXXXXXXXXXXXXX)\n"
#define DESC_GTF_ONT_PWD       "ONT password. <S><Length 1-10>\n"
#define DESC_GTF_ONT_LOID      "LOID user. <S><Length 1-24>\n"
#define DESC_GTF_ONT_LOID_PWD  "LOID password. <S><Length 1-12>\n"

#define DESC_GTF_PROFILE_NAME  "Profile name. <S><Length 1-16>\n"

#define DESC_GTF_LINE_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_LINE_PROFILE_ID   "Profile ID. <U><1-512>\n"
#define DESC_ETF_LLID_LIST         "LLID list. <L><1-8>(e.g.:1-2,3)\n"
#define DESC_GTF_TCONT_LIST        "T-CONT ID list. <L><1-3>(e.g.:1-2,3)\n"
#define DESC_GTF_TCONT_LIST_WITH_DEF "T-CONT ID list. <L><0-3>(e.g.:0-2,3)\n"

#define DESC_GTF_GEM_ID            "GEM ID. <U><1-24>\n"
#define DESC_GTF_TCONT_ID          "T-CONT ID. <U><1-3>\n"

#define DESC_GTF_SRV_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_SRV_PROFILE_ID   "Profile ID. <U><1-512>\n"
#define DESC_ONT_PORT_LIST         "ONT Port list. <L><1-24>(e.g.:1,3-5,8)\n"

#define DESC_ONU_ETH_PORT_ID        "ONT port ID. <U><1-8>\n"
#define DESC_ONU_POTS_PORT_ID       "ONT port ID. <U><1-8>\n"
#define DESC_ONU_CATV_PORT_ID       "ONT port ID. <U><1-8>\n"

#define DESC_ONT_PORT_ID           "ONT Port ID. <U><1-24>\n"
#define DESC_ONT_PORT_ALL          "All ports in the ONT\n"

#define DESC_GTF_CLASSIFICATION_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_CLASSIFICATION_PROFILE_ID   "Profile ID. <U><1-256>\n"

#define DESC_GTF_SLA_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_SLA_PROFILE_ID   "Profile ID. <U><1-256>\n"

#define DESC_GTF_ALARM_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_ALARM_PROFILE_ID   "Profile ID. <U><1-50>\n"
#define DESC_GTF_ALARM_PROFILE_THRESHOLD "Threshold <U><0-4294967294>\n"
#define DESC_GTF_ALARM_PROFILE_RTHRESHOLD "Restore threshold <U><0-4294967294>\n"

#define DESC_GTF_OPTICAL_ALARM_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_OPTICAL_ALARM_PROFILE_ID   "Profile ID. <U><1-256>\n"

#define DESC_GTF_DBA_PROFILE_NAME  "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_DBA_PROFILE_ID    "Profile ID. <U><1-128>\n"
#define DESC_GTF_DBA_PROFILE_ID_WITH_DEF "Profile ID. <U><0-128>\n"


#define DESC_GTF_SIPAGENT_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_SIPAGENT_PROFILE_ID   "Profile ID. <U><1-16>\n"
#define DESC_GTF_SIPAGENT_PROFILE_ID_WITH_DEF   "Profile ID. <U><0-16>\n"

#define DESC_GTF_POTS_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_POTS_PROFILE_ID   "Profile ID. <U><1-16>\n"
#define DESC_GTF_POTS_PROFILE_ID_WITH_DEF   "Profile ID. <U><0-16>\n"

#define DESC_GTF_DIGITMAP_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_DIGITMAP_PROFILE_ID   "Profile ID. <U><1-16>\n"
#define DESC_GTF_DIGITMAP_PROFILE_ID_WITH_DEF   "Profile ID. <U><0-16>\n"

#define DESC_GTF_SIPRIGHT_PROFILE_NAME "Profile name. <S><Length 1-16>\n"
#define DESC_GTF_SIPRIGHT_PROFILE_ID   "Profile ID. <U><1-16>\n"
#define DESC_GTF_SIPRIGHT_PROFILE_ID_WITH_DEF   "Profile ID. <U><0-16>\n"

#define DESC_GTF_TRAFFIC_PROFILE_ID   "Profile ID. <U><1-256>\n"
#define DESC_GTF_TRAFFIC_PROFILE_NAME "Profile name. <S><Length 1-16>\n"

#define DESC_PORT_RATE_CONFIG "Port rate configuration\n"

#define DESC_ACL_STR        "ACL configuration\n"
#define DESC_ACL_ID         "ACL ID. <U><2000-5999>\n"
#define DESC_ACL_INDEX      "ACL ID: <2000-2999>basic acl/<3000-4999>advanced acl/<5000-5999>link acl\n"
#define DESC_RULE_STR        "Rule ID configuration\n"
#define DESC_RULE_INDEX     "Rule ID. <U><1-16>\n"
#define RULE_STR "Rule configuration\n"
#define DESC_INBOUND "Apply the acl on inbound packets of the interface\n"
#define DESC_OUTBOUND "Apply the acl on outbound packets of the interface\n"
#define DESC_TRAFFIC_OUTERVLAN "Traffic outervlan configuration\n"
#define DESC_TRAFFIC_TRANSLATE "Traffic translate configuration\n"
#define DESC_RATE_VALUE     "Rate value. U<64-10240000>(Unit: Kbps)\n"
#define DESC_PORT           "Port ID\n"
#define DESC_DST_PORT           "Destination Port ID\n"
#define DESC_DSCP_V         "Dscp value: af1(10),af2(18),af3(26),af4(34),be(0),cs1(8),cs2(16),cs3(24),cs4(32),cs5(40),cs6(48),cs7(56),ef(46)\n"
#define DESC_PK             "Packet fileter configuration\n"
#define DESC_PORT_CPU       "For packet to cpu\n"
#define DESC_DSCP_LIST         "Dscp list. <L><0-63>(e.g.:1,3-5,8)\n"
#define DESC_8021P_V        "802.1p priority. <U><0-7>\n"
#define DESC_RULE_ALL  "All config acl rule\n"
#define DESC_RULE_PERMIT  "Permit acl rule\n"
#define DESC_RULE_DENY  "Deny acl rule\n"
#define DESC_RULE_SOURCEIP  "Source ip rule\n"
#define DESC_RULE_SOURCEIP_ADDRESS  "Source ip address\n"
#define DESC_RULE_SOURCEIP_MASK  "Wildcard-mask of source ip\n"
#define DESC_RULE_SOURCEIP_ANY  "Any source ip\n"
#define DESC_RULE_DESTIP  "Destination ip rule\n"
#define DESC_RULE_DESTIP_ADDRESS  "Destination ip address\n"
#define DESC_RULE_DESTIP_MASK  "Wildcard-mask of destination ip\n"
#define DESC_RULE_DESTIP_ANY  "Any Destination ip\n"
#define DESC_RULE_TIME_RANGE  "Set TimeRange\n"
#define DESC_RULE_TIME_RANGE_NAME  "Time range NAME\n"
#define DESC_RULE_SOURCEPORT  "Source port\n"
#define DESC_RULE_SOURCEPORT_LIST  "Source port: U<0-65535>. eg: 1 or 5-7\n"
#define DESC_RULE_DESTPORT  "Destination port\n"
#define DESC_RULE_DESTPORT_LIST  "Destination port: U<0-65535>. eg: 1 or 5-7\n"
#define DESC_RULE_PRECEDENCE "Priority precedence\n"
#define DESC_RULE_PRECEDENCE_PRI "precedence: 0(Routine),1(Priority),2(Immediate),3(Flash),4(Flash-override),5(Critical),6(Internetwork control),7(Network control)\n"
#define CMD_ACL_ID  "<2000-5999>"
#define CMD_ETF_ACL_ID  "<8000-8199>"
#define DESC_ETF_ACL_INDEX      "TF side acl. <U>"CMD_ETF_ACL_ID"\n"
#define DESC_INFO "description information"
#define DESC_QUEUE_ID      "Queue id. <U><0-7>\n"

#define DESC_IGMP          "<Group> igmp command group\n"

#define DESC_PORT_STATS_UPPER_THRESHOLD "Upper Threshold <U><0-4294967294>\n"
#define DESC_PORT_STATS_LOWER_THRESHOLD "Lower Threshold <U><0-4294967294>\n"

#define DESC_RSSI_THRESHOLD "Rssi Rx Threshold(-dbm): <2000-4000>\n"

#define CMD_FILE_NAME_LEN_MAX 64

#define VERSION_STR(major, minor, rev) "V" #major "." #minor "." #rev
#define VTY_VERSION VERSION_STR(1, 0, 1)
#define VTY_VER_STR_MAX_LEN  (sizeof("Vxxx.xxx.xxx") - 1)

#define CFG_HEADER_LINE_0           "#Saving user:"
#define CFG_HEADER_LINE_1           "#Saving time:"
#define CFG_HEADER_LINE_2           "#Version:"
#define CFG_HEADER_LINE_3           "#Product number:"
#define CFG_HEADER_LINE_4           "#Hardware version:"
#define CFG_HEADER_STR(x)           CFG_HEADER_LINE_##x

#endif /* _VTY_COMMON_H_ */

