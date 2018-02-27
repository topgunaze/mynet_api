/**************************************************************
 * �ļ�����:  tfEnvParamPub.h
 * ��    ��:  tonson tang
 * ��    ��:  2015.10.12
 * �ļ�����:  Env��������ͷ�ļ�
 * ��    ��:  V1.00
 * �޸���ʷ:  
 *     <�޸���>   <ʱ��>    <�汾 >   <����>
  
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
	UINT8	mac[6];						        /* MAC��ַ */
    char	license[32];		                /* ��Ȩ�� */
}PACKED IPC_ENV_MAC_LICENSE_T;


typedef struct
{
	UINT8	mac[6];						        /* MAC��ַ */
    char	model[TF_NVRAM_MODEL_LEN];		    /* ��Ʒ�ͺ� */
    char	sn[TF_NVRAM_SN_LEN];		        /* ��Ʒ���к� */
    char	vendor[TF_NVRAM_VENDOR_LEN];		/* ������Ϣ */
}PACKED IPC_ENV_PARAM_INFO_T;


/* disable structure packing */
#undef PACKED
#if !defined (__GNUC__)
#pragma pack()
#endif


#endif  /* __TF_NNV_PARAM_PUB_H__ */
