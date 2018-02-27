/**************************************************************
 * 文件名称:  tf_bit_map.h
 * 作           者:  keith.gong
 * 日           期:  2015.05.12
 * 文件描述:  位图处理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef __TF_BIT_MAP_H__
#define __TF_BIT_MAP_H__

#include "tf_types.h"

/*****************************************************************************
*
* bitListSet - Set a bit in the given byte array.
*/
inline void bitListSet    (    UINT8   * bitList,    UINT32     bit    );

/*****************************************************************************
*
*bitListClr - Remove a bit in the given byte array.
*/
inline void bitListClr    (    UINT8   * bitList,    UINT32     bit    );

/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
inline UINT8 bitListTst    (    UINT8   * bitList,    UINT32     bit    );

/*****************************************************************************
*
* bitListTstAll - Return >0 if a bit is set in the list, else zero.
*/
UINT32 bitListTstAll    (    UINT8   * bitList,    UINT32     bitListLen    );

#endif /* __TF_BIT_MAP_H__ */

