/**************************************************************
 * 文件名称:  sys_debug.h
 * 作           者:  keith.gong
 * 日           期:  2015.02.26
 * 文件描述:  控制工程调试信息打印
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#ifndef __SYS_DEBUG_H__
#define __SYS_DEBUG_H__

enum
{
    CLI_DEBUG_MOD0 = 0,
    CLI_DEBUG_MOD_MAX
};

#ifndef CLI_DEBUG
#define CLI_DEBUG(fmt, arg...) tflog_debug(CLI_DEBUG_MOD0, fmt, ##arg)
#endif

#endif /* __SYS_DEBUG_H__ */


