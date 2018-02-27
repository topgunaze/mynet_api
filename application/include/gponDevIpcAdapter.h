/**************************************************************
 * 文件名称:  gtfDevIpcAdapter.h
 * 作           者:  keith.gong
 * 日           期:  2015.05.30
 * 文件描述:  GTF  IPC  适配层
 * 版           本:  V1.00
 * 修改历史:
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef __GTF_DEV_IPC_ADAPTER_H__
#define __GTF_DEV_IPC_ADAPTER_H__

#pragma pack(1)

#define MANUFACTURE_MAX_MODULE_NUM 10 /* 每个模块厂商非默认模块个数 */

typedef struct SUPPORT_TRX_S
{
    char *p_manufacturer;
    UINT8 default_type;
    struct 
    {
        char *p_module_name;
        char *p_spec;
        char *p_version;
        UINT8 type;
    }module[MANUFACTURE_MAX_MODULE_NUM];
}SUPPORT_TRX_S;


#pragma pack()

#endif /* __GTF_DEV_IPC_ADAPTER_H__ */

