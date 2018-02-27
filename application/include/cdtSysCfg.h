/*
 *
 * panger123   
 *
 * 2014/02/27 - ?
 *
 */
#ifndef __TF_SYS_CFG_H__
#define __TF_SYS_CFG_H__

#include <tf_types.h>

//#define XINDE_VERSION

#define SYS_VERSION				"2.1.13"

#ifndef XINDE_VERSION
#define TF_SYS_VERSION			SYS_VERSION
#else
#define VERSION_PULS			"s"
#define TF_SYS_VERSION			SYS_VERSION VERSION_PULS
#endif

enum
{
    SYSTEM_CFG_RESTORING      = 1,
    SYSTEM_CFG_RESTORE_OK     = 2,
    SYSTEM_CFG_RESTORE_ERR    = 3,
    SYSTEM_CFG_RESTORE_WAIT   = 4
};

#define TF_L2_CFG_FILE         "l2.cfg"
#define TF_IGMP_CFG_FILE       "igmp.cfg"
#define TF_DHCP_CFG_FILE      "dhcp.cfg"
#define TF_RSTP_CFG_FILE       "rstp.cfg"
#define TF_TRUNK_CFG_FILE      "trunk.cfg"
#define TF_GTF_CFG_FILE       "gtf.cfg"
#define TF_SYSCTRL_CFG_FILE    "sysctrl.cfg"

/******************************* Sys Base Info *************************************/
#ifdef TF_1U4TF
#define TF_OLT_MODEL		    0x01070000
#else
#define TF_OLT_MODEL		    0x01060000	
#endif

#ifdef TF_1U4TF
#define CORP_TS_S 				"C-Data FD1104"
#else
#define CORP_TS_S 				"C-Data FD1108"
#endif
#define CORP_TS_L 				"C-Data Co.,Ltd."

#ifdef TF_1U4TF
#define TF_OLT_DEF_DESC		"CDATA FD1104 OLT"
#else
#define TF_OLT_DEF_DESC		"CDATA FD1108 OLT"
#endif
#define TF_MAX_DESC_LEN		256

#define TF_OLT_DEF_LOCATION	"Flat 6, Bldg 4,South 2 of Honghualing Industrial Zone, Liuxian Road, Xili Town, Shenzhen, Guangdong, China(518055)"
#define TF_MAX_LOCATION_LEN	512

#define TF_OLT_DEF_CONTACT		"marketing@cdatatec.com"
#define TF_MAX_CONTACT_LEN		256

#define TF_MAX_ALARM_DESC_LEN  128

#define TF_DEF_FTP_IP 			0xC0A80165
#define TF_DEF_FTP_USER		"admin"
#define TF_DEF_FTP_PASS		"admin"
#ifdef TF_1U4TF
#define TF_DEF_FTP_FILE		"FD1104S.image.gz"
#else
#define TF_DEF_FTP_FILE		"FD1108S.image.gz"
#endif

#ifdef TF_1U4TF
/***************/
/**  FD1104S  **/
/***************/
/******************************* Chassis Info *************************************/
#define TF_CHASSIC_TYPE 		0x01070101


/******************************* Card Module *************************************/
#define TF_MAIN_CARN_TYPE 		0x01070201

/******************************* Card Module *************************************/
#define TF_TF_CARD_TYPE 		0x01070301

#else
/***************/
/**  FD1108S  **/
/***************/
/******************************* Chassis Info *************************************/
#define TF_CHASSIC_TYPE 		0x01060101


/******************************* Card Module *************************************/
#define TF_MAIN_CARN_TYPE 		0x01060201

/******************************* Card Module *************************************/
#define TF_TF_CARD_TYPE 		0x01060302

#endif
typedef struct sysInfo_t
{
	UINT8										mac[6];
	UINT8										hardware[20];
	UINT8										serialNum[20];
}sysInfo_t;

typedef struct cliUser_t
{
	UINT8										cliUserName[20];
	UINT8										cliUserPassword[20];
}cliUser_t;

typedef struct tfSys_t
{
	UINT8										console;/*1--enable to output, 0--disable*/
	cliUser_t									cliUser;
	UINT8										sysDesc[TF_MAX_DESC_LEN];
	UINT8										sysLocation[TF_MAX_LOCATION_LEN];
	UINT8										sysContact[TF_MAX_CONTACT_LEN];
}tfSys_t;

tfSys_t	sysCfg;

#endif
