/**************************************************************
* 文件名称:  gtfOnuUg.h
* 作           者:  keith.gong
* 日           期:  2016.03.14
* 文件描述:  
* 版           本:  V1.00
* 修改历史:  
*     <修改者>     <时间>      <版本 >     <描述>
        keith.gong          2016.03.14      v1.00              Create
**************************************************************/


#ifndef __GTF_ONUUG_H__
#define __GTF_ONUUG_H__


#include "tf_types.h"
#include "sys_common.h"

#pragma pack(1)

#define FTP_SRV_IP_LEN                  16
#define FTP_NAME_LEN                    32
#define FTP_PWD_LEN                     32
#define ONT_LOAD_FILENAME_LEN           80

#define MIN_LOAD_FILE_SIZE              (1024)          /*1KB*/
#define MAX_LOAD_FILE_SIZE              (1024*1024*30)  /*30MB*/

#define ONT_LOAD_FILE_LOCAL_FULL_NAME   "/var/ont_load_file"

typedef enum
{
    ONT_LOAD_STATUS_INIT,
    ONT_LOAD_STATUS_FAILED,
    ONT_LOAD_STATUS_SUCCESS,
    ONT_LOAD_STATUS_DOWNLOADING
}
ONT_LOAD_STATUS;

typedef enum
{
    ONT_LOAD_ACTIVE_IMMEDIATE,
    ONT_LOAD_ACTIVE_NEXTSTARTUP,
    ONT_LOAD_ACTIVE_GRACEFUL,

    ONT_LOAD_ACTIVE_MODE_NUM_OF
}
ONT_ACTIVE_MODE;

typedef struct
{
    char        ftp_name[FTP_NAME_LEN];
    char        ftp_pwd[FTP_PWD_LEN];
    char        ftp_ip[FTP_SRV_IP_LEN];
    UINT16      ftp_port;
    char        file_name[ONT_LOAD_FILENAME_LEN]; 
}
ONT_LOAD_FTP_INFO;

typedef struct
{
    char                protocol_type; /*0:ftp*/
    ONT_LOAD_FTP_INFO   load_ftp_info;
    UINT8               loading_flag;
    UINT8               active_mode;
    UINT8               result;
    UINT8               bind_bitmap[SYS_MAX_TF_NUM][SYS_MAX_ONU_NUM/8];
}
GTF_ONT_LOAD_INFO;

typedef struct
{
    FILE   *p_fp;
    UINT32 file_size;
    UINT8  *p_file_buf;
    UINT32 crc;
    INT32  lock;
}ONT_IMAGE_UPDATE_INFO;


/*一个窗口不能超过256个分段*/
#define WINDOWS_SECTION_NUM 256 

/*一个分段31字节，WINDOWS_SECTION_NUM个段划分一个窗口*/
#define WINDOWS_TOTAL_SIZE ((DEFAULT_OMCI_SOFTWARE_DOWNLOAD_SECTION_SIZE) * (WINDOWS_SECTION_NUM)) 

#define MAX_WINDOW_RESEND_NUM    3

#pragma pack()

#endif /* __GTF_ONUUG_H__ */

