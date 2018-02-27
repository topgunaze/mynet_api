/**************************************************************
 * 文件名称:  tf_bit_map.c
 * 作           者:  keith.gong
 * 日           期:  2015.05.12
 * 文件描述:  位图处理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/


#include "tf_bit_map.h"


static const UINT8 bitMask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

/*****************************************************************************
*
* bitListSet - Set a bit in the given byte array.
*/
inline void bitListSet    (    UINT8   * bitList,    UINT32     bit    )
{
    bitList[bit/8] |= bitMask[bit%8];
}

/*****************************************************************************
*
*bitListClr - Remove a bit in the given byte array.
*/
inline void bitListClr    (    UINT8   * bitList,    UINT32     bit    )
{    
    bitList[bit/8] &= ~bitMask[bit%8];
}

/*****************************************************************************
*
* bitListTst - Return True if the bit is set, else False.
*/
inline UINT8 bitListTst    (    UINT8   * bitList,    UINT32     bit    )
{    
    return (bitList[bit/8] & bitMask[bit%8])? 1 : 0;
}

/*****************************************************************************
*
* bitListTstAll - Return >0 if a bit is set in the list, else zero.
*/
UINT32 bitListTstAll    (    UINT8   * bitList,    UINT32     bitListLen    )
{    
    UINT32 i, found = 0, byteCount;
    
    /* Figure out the real number of bytes to scan */
    byteCount = ((bitListLen%8) == 0) ? (bitListLen/8) : (bitListLen/8) + 1;    
    
    /* Add up all of the bytes to determine if the bitList is empty. */    
    for( i = 0; i<byteCount; i++ )        
        found += bitList[i];
    
    return found;
}

