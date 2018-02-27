/**************************************************************
 * 文件名称:  tfEnvParamPub.h
 * 作    者:  tonson tang
 * 日    期:  2015.10.12
 * 文件描述:  Env参数公共头文件
 * 版    本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
  
**************************************************************/

#ifndef __TF_NNV_PARAM_PUB_H__
#define __TF_NNV_PARAM_PUB_H__

#include "tf_flash_images.h"

/* enable structure packing */
#if defined (__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#pragma pack(1)
#define	PACKED
#endif

typedef struct
{
	UINT8	mac[6];						        /* MAC地址 */
    char	license[32];		                /* 授权码 */
}PACKED IPC_ENV_MAC_LICENSE_T;


typedef struct
{
	UINT8	mac[6];						        /* MAC地址 */
    char	model[TF_NVRAM_MODEL_LEN];		    /* 产品型号 */
    char	sn[TF_NVRAM_SN_LEN];		        /* 产品序列号 */
    char	vendor[TF_NVRAM_VENDOR_LEN];		/* 厂商信息 */
}PACKED IPC_ENV_PARAM_INFO_T;


/* disable structure packing */
#undef PACKED
#if !defined (__GNUC__)
#pragma pack()
#endif


#endif  /* __TF_NNV_PARAM_PUB_H__ */

