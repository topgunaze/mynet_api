/**************************************************************
 * 文件名称:  tf_string_opera.c
 * 作           者:  keith.gong
 * 日           期:  2015.05.12
 * 文件描述:  字符串处理
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/


#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>

#include "tf_types.h"


/**************************************************************
 * 函数名称: tf_str_2_id_array
 * 作           者: keith.gong
 * 日           期: 2015年03月30日
 *
 * 函数功能: 
                            将字符串(1-7,2,9)转换到数组中；
                            
 * 输入参数: 
                            char  *str, 
                            unsigned int *idArry, 
                            unsigned int  idNum,
                            unsigned int  idMin,
                            unsigned int  idMax,
 * 输出参数: 
                            unsigned int *idNumGet
 * 返回数值: 
                            成功返回0
 * 其           它: 
                            注意，该函数会修改str
**************************************************************/
int 
tf_str_2_id_array(
                char  *str, 
                unsigned int *pIdArray, 
                const unsigned int idNum,
                const unsigned int idMin,
                const unsigned int idMax,
                unsigned int *pGetIdNum)
{
    char *p;
    unsigned int index, count = 0, pstart, pend;
    unsigned int end, start, buf[4095] = {0};

    if(pIdArray == NULL || str == NULL || idNum > sizeof(buf)/sizeof(buf[0]))
        return -1;  /* 参数错误*/

    p = strtok(str, ", ");
    while(p)
    {
        if((start = atoi(p)) < idMin || start > idMax)
            return -1; /* 输入错误*/
        
        /* 插入排序*/
        for(index = 0; index < count; index++)
        {
            if(buf[index] > start)
            {
                if(count + 1 > idNum)
                    return -2;  /* 表满*/

                memmove(buf + index + 1, buf + index, (count - index) * sizeof(buf[0]));
                buf[index] = start;
                count++;
                break;
            }
            else if(buf[index] == start)
                break;
        }

        /* 最大值，表位插入*/
        if(index == count)
        {
            if(count + 1 > idNum)
                return -2;  /* 表满*/
            
            buf[count++] = start;
        }

        while(*++p)
        {
            if(*p == '-')
            {          
                if((end = atoi(++p)) < idMin || end < start || end > idMax)
                    return -1;  /* 输入错误*/

                pstart = pend = 0;
                for(index = 0; index < count; index++)
                {
                    if(buf[index] == start)
                    {
                        pstart = index;
                    }
                    
                    if(buf[index] > end)
                    {
                        pend = index;
                        break;
                    }
                }
                
                if(index != count)
                {
                    if(count + count - pend > idNum)
                        return -2;  /* 表满*/
                    
                    memmove(buf + pstart + end - start + 1, buf + pend, (count - pend) * sizeof(buf[0]));
                }

                index = pstart;
                count += (end-start) - ((pend?pend:count) - pstart - 1);
                if(count > idNum)
                    return -2;  /* 表满*/

                while(start < end) 
                    buf[++index] = ++start;
            }
            else if(*p > '9' || *p < '0')
                return -1;  /* 输入错误*/

        }
        p = strtok(NULL, ", ");
    }

    memcpy(pIdArray, buf, count * sizeof(buf[0]));
    *pGetIdNum = count;

    return 0;
}


/**************************************************************
 * 函数名称: port_id_array_2_str
 * 作           者: keith.gong
 * 日           期: 2015年05月26日
 *
 * 函数功能: 
                            将数组转换为字符串(1,2,5-7)
                            
 * 输入参数: 
                            const unsigned int *pIdArray,      数组指针
                            const unsigned int idNum,            数组元素个数
                            const unsigned char sortFlag,       排序标识
                            const unsigned int strLen,            输出字符串缓冲区长度
                            const unsigned char * prefix,       输出字符串前缀，若不使用可输入形参"" 。
 * 输出参数: 
                            unsigned char * const str,            输出字符串缓冲区
 * 返回数值: 
                            成功返回0
 * 其           它: 
                            注意，该函数未检查数组元素合法性!!
**************************************************************/
int 
port_id_array_2_str(
                const unsigned int *pIdArray, 
                const unsigned int idNum,
                const unsigned char sortFlag,
                unsigned char * const str,
                const unsigned int strLen,
                const char * prefix)

{
    unsigned int index;
    unsigned char buf[4096];
    unsigned char offset;
    unsigned int end, start;
    int diff;

    if(pIdArray == NULL || idNum < 1 || str == NULL || strLen > sizeof(buf))
        return -1;  /* 参数错误*/

    if(sortFlag)
    {
        /* kth.debug  添加排序代码*/
        return -2;
    }

    start = pIdArray[0];
    offset = snprintf(buf, sizeof(buf), "%s%d", prefix, start);
    for(index = 1; index < idNum; index++)
    {
        if(pIdArray[index-1] == pIdArray[index] - 1)
            continue;

        end = pIdArray[index-1];
        diff = end - start;
        if(diff == 1)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, ",%s%d,%s%d", prefix, end, prefix, pIdArray[index]);
        else if(diff > 1)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, "-%s%d,%s%d", prefix, end, prefix, pIdArray[index]);  
        else if(diff == 0)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, ",%s%d", prefix, pIdArray[index]);  
        start = pIdArray[index];
    }
    end = pIdArray[index-1];
    if(start != end)
    {
        diff = end - start;
        if(diff == 1)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, ",%s%d", prefix, end);
        else if(diff > 1)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, "-%s%d", prefix, end);  
        else if(diff == 0)
            offset += snprintf(&buf[0]+offset, sizeof(buf)-offset, ",%s%d", prefix, end);  
        start = pIdArray[index];	
    }
    
    snprintf(str, strLen, "%s", buf);

    return 0;
}

/*************************************************************
 * ltrim - 删除左边的空格
 * 
 * str[in]: 需要去除空的的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *ltrim(char *str)
{
    if (str == NULL)
        return NULL;

    while(*str != '\0' && *str == ' ')
        str++;

    return str;
}

/*************************************************************
 * rtrim - 删除右边的空格
 * 注意:会修改str内容
 * 
 * str[in]: 需要去除空的的源字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *rtrim(char *str)
{
    char *end = NULL;
    
    if (str == NULL)
        return NULL;

    end = str + strlen(str) - 1;
    while(str != end && *end == ' ')
    {
        *end = '\0';
        end--;
    }

    return str;
}

/*************************************************************
 * lstripNewLine - 删除左边的换行符
 * 
 * str[in]: 需要去除换行符的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *lstripNewLine(char *str)
{
    if (str == NULL)
        return NULL;

    while((*str != '\0') && (*str == '\r' || *str == '\n'))
    {
        str++;
    }

    return str;
}

/*************************************************************
 * rstripNewLine - 删除右边的换行符
 * 注意:会修改str内容
 * 
 * str[in]: 需要去除换行符的字符串
 *
 * Returns: 返回指向原字符串数据区的指针,如果参数错误,返回NULL;
 *
 ************************************************************/
char *rstripNewLine(char *str)
{
    char *end = NULL;
    
    if (str == NULL)
        return NULL;

    end = str + strlen(str) - 1;
    while((end != str) && (*end == '\r' || *end == '\n'))
    {
        *end = '\0';
        end--;
    }

    return str;
}


