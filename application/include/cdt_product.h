/******************************************************************************
Copyright (C), 2014-2024, C-Data Tech. Co., Ltd.
  文 件 名   : tf_product.h
  作    者   : jh.liang
  版 本 号   : 1.0.0 
  生成日期   : 2015年3月13日
  功能描述   : 定义产品相关的属性,包括产品的端口，芯片平台，sdk等相关的属性
******************************************************************************/
#ifndef IPC_TF_PRODUCT_H
#define IPC_TF_PRODUCT_H

#define ETHERADDR_LEN               6

#define SWITCH_MIRROR_VLAN_NUM		8
#define	MAX_VLAN_NUM				4094
#define SYS_DEF_IP					0xC0A80164 /*192.168.1.100*/
#define SYS_DEF_NETMASK				0xFFFFFF00 /*255.255.255.0*/
#define SYS_DEF_GATEWAY				0xC0A80101 /*192.168.1.1*/
#define SYS_DEF_MGMT_VLAN			1
#define SYSINFO_PATH "/mnt/sysInfo.dat"

#define TF_FD1616_16

enum{
    SDK_VER_NONE,
    SDK_VER_5_10_2,
    SDK_VER_6_3_8,
    SDK_VER_6_4_8,

    SDK_VER_END
};


#define AGGR_MAX_PORT_PER_GROUP     8
#if defined TF_FD1008_4
#define MAX_AGGR_GROUP				2
#define TF_SW_MAX_PORTCNT   		4
#define TF_FB_MAX_PORTCNT   		8
#define SDK_VER                     SDK_VER_5_10_2
#define PLATFORM_BCM53314
#elif defined (TF_FD1008_8)
#define MAX_AGGR_GROUP				4
#define TF_SW_MAX_PORTCNT   		8
#define TF_FB_MAX_PORTCNT   		16
#define SDK_VER                     SDK_VER_5_10_2
#define PLATFORM_BCM53314
#elif defined (TF_FD1508_8)
#define MAX_AGGR_GROUP				8
#define TF_SW_MAX_PORTCNT   		19
#define TF_FB_MAX_PORTCNT   		27
#define PLATFORM_BCM55440
#define SDK_VER                     SDK_VER_6_3_8
#define RSTP_TRUNK
#elif defined (TF_FD3000_40)
#define MAX_AGGR_GROUP				8
#define TF_SW_MAX_PORTCNT   		19
#define TF_FB_MAX_PORTCNT   		27
#define PLATFORM_BCM56334
#elif defined (TF_FD1616_16)
#define MAX_AGGR_GROUP				8
#define TF_SW_MAX_PORTCNT   		10
#define TF_FB_MAX_PORTCNT   		16
#define PLATFORM_BCM56452
#define SDK_VER                     SDK_VER_6_4_8
#endif

#if defined PLATFORM_BCM53314
#define SWITCH_PORT_NUM             24
#define SWITCH_GEPORT_NUM           24
#define SWITCH_HGPORT_NUM           0
#define SWITCH_XEPORT_NUM           0
#elif defined PLATFORM_BCM55440
#define SWITCH_PORT_NUM             27
#define SWITCH_GEPORT_NUM           17
#define SWITCH_HGPORT_NUM           2
#define SWITCH_XEPORT_NUM           8
#elif defined PLATFORM_BCM56334
#define SWITCH_PORT_NUM             50
#define SWITCH_GEPORT_NUM           48
#define SWITCH_HGPORT_NUM           2
#define SWITCH_XEPORT_NUM           0
#elif defined PLATFORM_BCM56452
#define SWITCH_PORT_NUM             50
#define SWITCH_GEPORT_NUM           40
#define SWITCH_HGPORT_NUM           0
#define SWITCH_XEPORT_NUM           2

#endif

#define RSTP_MAX_PORT_NUM 	    	64//(SWITCH_GEPORT_NUM+SWITCH_HGPORT_NUM)
#define INVALID_TRUNK_ID        	-1

#ifdef PLATFORM_BCM55440
/*把用户所认识的端口号转换为底层硬件的端口号*/
#define port_user2sdk(num) ((num>17)?(num-1+8):(num))
/*把底层读到的端口号转换为用户认识的端口号*/
#define port_sdk2user(port) ((port>24)?(port-8+1):(port))

#define pbmp_user2sdk(userpbmp,sdkpbmp)\
do{\
    sdkpbmp.pbits[0] = ((((userpbmp<1) & 0xfe0000) << 9)|((userpbmp<<1) & 0x3fffe));\
    sdkpbmp.pbits[1] = ((((userpbmp<1) & 0x7000000) >> 24));\
}while(0)

#define pbmp_sdk2user(sdkpbmp,userpbmp)\
do{\
    userpbmp = (((sdkpbmp.pbits[0]>>1)&0x3ffff));\
    userpbmp = userpbmp|((((sdkpbmp.pbits[1]>>1) & 0x7) << 24));\
}while(0)

#endif

#ifdef PLATFORM_BCM53314
#define port_user2sdk(num) (num)
#define port_sdk2user(port) (port)

#define pbmp_user2sdk(userpbmp,sdkpbmp)\
do{\
    sdkpbmp.pbits[0] = (((userpbmp<<1) & 0xfffffffe));\
    sdkpbmp.pbits[1] = 0x0000000;\
}while(0)

#define pbmp_sdk2user(sdkpbmp,userpbmp)\
do{\
    userpbmp = (((sdkpbmp.pbits[0]>>1)&0xffffffff));\
}while(0)
#endif

#endif //IPC_TF_PRODUCT_H


