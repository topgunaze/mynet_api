/**************************************************************
* 文件名称:  
* 文件描述:
* 版           本:  
* 修改历史:
* <修改者>     <时间>      <版本 >     <描述>
**************************************************************/

#ifndef __FK_COMMON_H__
#define __FK_COMMON_H__

int
fk_onu_state_machine_cb_call(
                const uint8_t port_id,
                const uint16_t onu_id,
                const uint16_t phy_id,
                const uint8_t  type);

#endif

