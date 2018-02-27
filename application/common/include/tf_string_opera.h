/**************************************************************
 * 文件名称:  tf_string_opera.h
 * 作           者:  keith.gong
 * 日           期:  2015.05.12
 * 文件描述:  字符串处理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>     <时间>      <版本 >   <描述>
        keith.gong          2015.05.12      V1.00           Create
**************************************************************/

#ifndef __TF_STRING_OPERA_H__
#define __TF_STRING_OPERA_H__

int 
tf_str_2_id_array(
                char  *str, 
                unsigned int *pIdArray, 
                const unsigned int idNum,
                const unsigned int idMin,
                const unsigned int idMax,
                unsigned int *pGetIdNum);

int 
port_id_array_2_str(
                const unsigned int *pIdArray, 
                const unsigned int idNum,
                const unsigned char sortFlag,
                unsigned char * const str,
                const unsigned int strLen,
                const char * prefix);


char *ltrim(char *str);
char *rtrim(char *str);
char *lstripNewLine(char *str);
char *rstripNewLine(char *str);

#endif


