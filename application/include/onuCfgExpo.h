/**************************************************************
 * 文件名称:  onuCfgExpo.h
 * 作           者:  keith.gong
 * 日           期:  2015.05.22
 * 文件描述:  ONU 配置模块系统层头文件
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef __ONU_CFG_EXPO_H__
#define __ONU_CFG_EXPO_H__


typedef enum
{
    ONUCFG_NO_ERROR,

    /* profile */
    ONUCFG_ERROR_PROFILE_ID_OUT_OF_RANGE,
    ONUCFG_ERROR_PROFILE_TABLE_FULL,
    ONUCFG_ERROR_PROFILE_EXIST,
    ONUCFG_ERROR_PROFILE_NOT_EXIST,
    ONUCFG_ERROR_PROFILE_NOT_FOUND,
    ONUCFG_ERROR_PROFILE_NAME_EXIST,
    ONUCFG_ERROR_PROFILE_MALLOC_FAIL,
    ONUCFG_ERROR_PROFILE_IN_BIND_STATE,
    
    ONUCFG_ERROR_INVALID_ETH_PORT_NUM,
    ONUCFG_ERROR_INVALID_POTS_PORT_NUM,
    ONUCFG_ERROR_INVALID_MOCA_PORT_NUM,
    ONUCFG_ERROR_INVALID_CATV_PORT_NUM,
    
    ONUCFG_ERROR_TF_ID_OUT_OF_RANGE,
    ONUCFG_ERROR_ONU_ID_OUT_OF_RANGE,    
    ONUCFG_ERROR_ETH_ID_OUT_OF_RANGE,
    
    ONUCFG_ERROR_PARAM_ERROR,
}ONUCFG_ERROR_DTE;

typedef enum
{
    ONU_CFG_RESULT_INIT,
    ONU_CFG_RESULT_SUCCESS,
    ONU_CFG_RESULT_FAILED,
}ONU_CFG_RESULT_DTE;


/**************************************************************
 * 函数名称: Cdt_Gtf_OnuCfg_SrvProfileBindSingleAdd
 * 作           者: keith.gong
 * 日           期: 2015年04月22日
 *
 * 函数功能: 
                            绑定Profile
 * 输入参数: 
 * 输出参数: 
 * 返回数值: 
 * 其           它: 
**************************************************************/
RTN_STATUS 
Cdt_Gtf_OnuCfg_SrvProfileBindSingleAdd(
                UINT32 profile_id, 
                UINT32 tfId, 
                UINT32 onuId);

RTN_STATUS 
Cdt_Gtf_OnuCfg_SrvProfileBindSingleDel(
                UINT32 profile_id, 
                UINT32 tfId, 
                UINT32 onuId);


#endif /* __ONU_CFG_EXPO_H__ */

