/**************************************************************
* 文件名称: 
* 文件描述:
* 版           本: 
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>

**************************************************************/

#include "tf_adaptor_errno.h"

#if DEFUNC("全局错误码转换接口")

unsigned int 
tf_rc2info_convert(unsigned int rc_val, tf_rc_info *p_rc_info)
{
    if (!p_rc_info)
    {
        return TF_MDW_NET_RC_PARAM_NULL;
    }
    
    if (rc_val>=TF_RETURNCODE_MAX_VALUE)
    {
        return TF_MDW_NET_RC_PARAM_OUT_OF_RANGE;
    }
    
    p_rc_info->obj_id = (rc_val/TF_RETURNCODE_BASE)/TF_RETURNCODE_BASE;

    switch(p_rc_info->obj_id)
    {
        case OBJ_DRV_SDK:case OBJ_MDW_LIBEV:case OBJ_MDW_SQL:
        case OBJ_MDW_NET:
        case OBJ_APP_ALARM:
            {
                //暂未使用 
            }
            break;

        case OBJ_DRV_ZK:
            {
                p_rc_info->sub_obj_id.olt_id = (rc_val/TF_RETURNCODE_BASE)%TF_RETURNCODE_BASE;
            }
            break;

        case OBJ_DRV_FK:
            {
                p_rc_info->sub_obj_id.onu_id = (rc_val/TF_RETURNCODE_BASE)%TF_RETURNCODE_BASE;
            }
            break;

        default:
            printf("obj id: %d not in range!!!\n", p_rc_info->obj_id);
            return TF_MDW_NET_RC_PARAM_OUT_OF_RANGE;
    }

    p_rc_info->err_id = rc_val%TF_RETURNCODE_BASE;
  
    return 0;
}

unsigned int 
tf_info2rc_convert(unsigned int *p_rc_val, tf_rc_info *p_rc_info)
{
    if (!p_rc_val || !p_rc_info)
    {
        return TF_MDW_NET_RC_PARAM_NULL;
    }

    if (p_rc_info->obj_id >= TF_RETURNCODE_BASE || p_rc_info->err_id >= TF_RETURNCODE_BASE || 
        *(unsigned int*)&p_rc_info->sub_obj_id >= TF_RETURNCODE_BASE)
    {
        return TF_MDW_NET_RC_PARAM_OUT_OF_RANGE;
    }

    *p_rc_val = p_rc_info->err_id;
    *p_rc_val += p_rc_info->obj_id*TF_RETURNCODE_BASE*TF_RETURNCODE_BASE;
    
    switch(p_rc_info->obj_id)
    {
        case OBJ_DRV_SDK:case OBJ_MDW_LIBEV:case OBJ_MDW_SQL:
        case OBJ_MDW_NET:
            {
                //暂未使用 
            }
            break;
        
        case OBJ_APP_ZK:
           {
               *p_rc_val += p_rc_info->sub_obj_id.olt_id * TF_RETURNCODE_BASE;
           }
           break;    
        
        case OBJ_APP_ALARM:
           {
               *p_rc_val += p_rc_info->sub_obj_id.profile_id * TF_RETURNCODE_BASE;
           }
           break;
        
           default:
               printf("obj id: %d not in range!!!\n", p_rc_info->obj_id);
               return TF_MDW_NET_RC_PARAM_OUT_OF_RANGE;
    }

    return 0;
}

#endif

