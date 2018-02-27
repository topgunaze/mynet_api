/**************************************************************
* 文件名称: 
* 文件描述:
* 版           本: 
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>

**************************************************************/

#ifndef __TF_ADAPTOR_ERRNO__
#define __TF_ADAPTOR_ERRNO__

#include "tf_adaptor_common_if.h"


#if DEFUNC("全局错误码枚举")

typedef enum
{
    FK_SDK_RC_OK                                 = 0,   /**< OK */
    FK_SDK_RC_IN_PROGRESS                        = 1,   /**< Operation is in progress */
    FK_SDK_RC_PARM                               = 2,   /**< Error in parameters */
    FK_SDK_RC_NOMEM                              = 3,   /**< No memory */
    FK_SDK_RC_NORES                              = 4,   /**< No resources */
    FK_SDK_RC_INTERNAL                           = 5,   /**< Internal error */
    FK_SDK_RC_NOENT                              = 6,   /**< Entry doesn't exist */
    FK_SDK_RC_NODEV                              = 7,   /**< Device doesn't exist */
    FK_SDK_RC_ALREADY                            = 8,   /**< Entry already exists */
    FK_SDK_RC_RANGE                              = 9,   /**< Out of range */
    FK_SDK_RC_PERM                               = 10,  /**< No permission to perform an operation */
    FK_SDK_RC_NOT_SUPPORTED                      = 11,  /**< Operation is not supported */
    FK_SDK_RC_PARSE                              = 12,  /**< Parsing error */
    FK_SDK_RC_INVALID_OP                         = 13,  /**< Invalid operation */
    FK_SDK_RC_IO                                 = 14,  /**< I/O error */
    FK_SDK_RC_STATE                              = 15,  /**< Object is in bad state */
    FK_SDK_RC_DELETED                            = 16,  /**< Object is deleted */
    FK_SDK_RC_TOO_MANY                           = 17,  /**< Too many objects */
    FK_SDK_RC_NO_MORE                            = 18,  /**< No more entries */
    FK_SDK_RC_OVERFLOW                           = 19,  /**< Buffer overflow */
    FK_SDK_RC_COMM_FAIL                          = 20,  /**< Communication failure */
    FK_SDK_RC_NOT_CONNECTED                      = 21,  /**< No connection with the target system */
    FK_SDK_RC_SYSCALL_ERR                        = 22,  /**< System call returned error */
    FK_SDK_RC_MSG_ERROR                          = 23,  /**< Received message is insane */
    FK_SDK_RC_TOO_MANY_REQS                      = 24,  /**< Too many outstanding requests */
    FK_SDK_RC_TIMEOUT                            = 25,  /**< Operation timed out */
    FK_SDK_RC_TOO_MANY_FRAGS                     = 26,  /**< Too many fragments */
    FK_SDK_RC_NULL                               = 27,  /**< Got NULL pointer */
    FK_SDK_RC_READ_ONLY                          = 28,  /**< Attempt to set read-only parameter */
    FK_SDK_RC_ONU_ERR_RESP                       = 29,  /**< ONU returned an error restfse */
    FK_SDK_RC_MANDATORY_PARM_IS_MISSING          = 30,  /**< Mandatory parameter is missing */
    FK_SDK_RC_KEY_RANGE                          = 31,  /**< Key field was out of range */
    FK_SDK_RC_QUEUE_EMPTY                        = 32,  /**< Rx PCIe queue empty */
    FK_SDK_RC_QUEUE_FULL                         = 33,  /**< Tx PCIe queue full */
    FK_SDK_RC_TOO_LONG                           = 34,  /**< Processing is taking too long, but will finish eventually */
    FK_SDK_RC_INSUFFICIENT_LIST_MEM              = 35,  /**< Not enough memory was provided for variable-length lists */
    FK_SDK_RC_OUT_OF_SYNC                        = 36,  /**< Sequence number or operation step was out of sync. */
    FK_SDK_RC_CHECKSUM                           = 37,  /**< Checksum error */
    FK_SDK_RC_IMAGE_TYPE                         = 38,  /**< Unsupported file/image type */
    FK_SDK_RC_INCOMPLETE_TERMINATION             = 39,  /**< Incomplete premature termination */
} fk_sdk_errno;

typedef enum
{
    FK_ZK_RC_OK = 0,

    FK_ZK_RC_NET_INIT,
    FK_ZK_RC_NET_NOT_CONNECT,
    FK_ZK_RC_NET_NOT_WORK,
    FK_ZK_RC_TASK_CREATE,
    FK_ZK_RC_MODULE_INIT,
    FK_ZK_RC_OMCI_FUNC_REG,

    FK_ZK_RC_MSG_SYN_VARIFY,
    FK_ZK_RC_PARAM_GET,
    FK_ZK_RC_MSG_SYN_WAITE,

    FK_ZK_RC_PORT_CFG_AND_ACTIVE,//10
    FK_ZK_RC_AUTO_INDICATION_OUT_OF_RANGE,
    FK_ZK_RC_PROXY_INDICATION_OUT_OF_RANGE,

    FK_ZK_RC_MEM_ALLOCATION,
    FK_ZK_RC_PARAM_NULL,
    FK_ZK_RC_PARAM_OUT_OF_RANGE,
    
    FK_ZK_RC_MSG_QUEUE_CREATE,
    FK_ZK_RC_MSG_QUEQUE_PUT,
    FK_ZK_RC_MSG_PACK,
    FK_ZK_RC_MSG_CMD_UNKNOWN,

    FK_ZK_RC_NET_BIND,
    FK_ZK_RC_NET_CONNCET,

    FK_ZK_RC_OPERATION_BUSY,
    FK_ZK_RC_OPERATION_TIMEOUT,
    FK_ZK_RC_OPERATION_UNREASONABLE,

    FK_ZK_RC_SQL_DB_INIT,
    FK_ZK_RC_SQL_DB_NOT_FOUND,
    
}fk_zk_errno;

typedef enum
{
    FK_FK_RC_OK = 0, 

    FK_FK_RC_PARAM_NULL,
    FK_FK_RC_PARAM_OUT_OF_RANGE,

    FK_FK_RC_TASK_CREATE,

    FK_FK_RC_OPERATION_BUSY,
    FK_FK_RC_OPERATION_TIMEOUT,
    FK_FK_RC_OPERATION_UNREASONABLE,
    
}fk_fk_errno;

typedef enum
{
    TF_MDW_NET_RC_OK = 0,
    
    TF_MDW_NET_RC_ERROR,
    TF_MDW_NET_RC_PARAM_NULL,
    TF_MDW_NET_RC_PARAM_OUT_OF_RANGE,
    TF_MDW_NET_RC_MEM_ALLOCATE,

    TF_MDW_NET_RC_REC_PACKET,
    TF_MDW_NET_RC_REMOTE_DISCONNECT,
    TF_MDW_NET_RC_LOCAL_DISCONNECT,
    TF_MDW_NET_RC_SETSOCKOPT,
    TF_MDW_NET_RC_GETSOCKOPT,
    TF_MDW_NET_RC_FCNTL,
    TF_MDW_NET_RC_INIT,
    TF_MDW_NET_RC_MSG_HEADER_VERIFY,

    TF_MDW_NET_RC_MSG_SEM_WAIT,
    TF_MDW_NET_RC_MSG_CMD_ID_OUT_OF_RANGE,

    TF_MDW_NET_RC_MUTEX_CREATE,
    TF_MDW_NET_RC_RWLOCK_CREATE,
    
    TF_MDW_NET_RC_COMPRESS_NO_NEED,
    TF_MDW_NET_RC_UNCOMPRESS_NO_NEED,
    TF_MDW_NET_RC_COMPRESS_FAIL,
    TF_MDW_NET_RC_UNCOMPRESS_FAIL,
    
}tf_mdw_net_errno;

typedef enum
{
    TF_MDW_LIBEV_RC_OK = 0,
    
}tf_mdw_libev_errno;

typedef enum
{
    TF_MDW_SQL_RC_OK = 0,           /* Successful result */
    TF_MDW_SQL_RC_ERROR,         /* SQL error or missing database */
    TF_MDW_SQL_RC_INTERNAL,      /* Internal logic error in SQLite */
    TF_MDW_SQL_RC_PERM,          /* Access permission denied */
    TF_MDW_SQL_RC_ABORT,         /* Callback routine requested an abort */
    TF_MDW_SQL_RC_BUSY,          /* The database file is locked */
    TF_MDW_SQL_RC_LOCKED,        /* A table in the database is locked */
    TF_MDW_SQL_RC_NOMEM,         /* A malloc() failed */
    TF_MDW_SQL_RC_READONLY,      /* Attempt to write a readonly database */
    TF_MDW_SQL_RC_INTERRUPT,     /* Operation terminated by sqlite3_interrupt()*/
    TF_MDW_SQL_RC_IOERR,         /* Some kind of disk I/O error occurred */
    TF_MDW_SQL_RC_CORRUPT,       /* The database disk image is malformed */
    TF_MDW_SQL_RC_NOTFOUND,      /* Unknown opcode in sqlite3_file_control() */
    TF_MDW_SQL_RC_FULL,          /* Insertion failed because database is full */
    TF_MDW_SQL_RC_CANTOPEN,      /* Unable to open the database file */
    TF_MDW_SQL_RC_PROTOCOL,      /* Database lock protocol error */
    TF_MDW_SQL_RC_EMPTY,         /* Database is empty */
    TF_MDW_SQL_RC_SCHEMA,        /* The database schema changed */
    TF_MDW_SQL_RC_TOOBIG,        /* String or BLOB exceeds size limit */
    TF_MDW_SQL_RC_CONSTRAINT,    /* Abort due to constraint violation */
    TF_MDW_SQL_RC_MISMATCH,      /* Data type mismatch */
    TF_MDW_SQL_RC_MISUSE,        /* Library used incorrectly */
    TF_MDW_SQL_RC_NOLFS,         /* Uses OS features not supported on host */
    TF_MDW_SQL_RC_AUTH,          /* Authorization denied */
    TF_MDW_SQL_RC_FORMAT,        /* Auxiliary database format error */
    TF_MDW_SQL_RC_RANGE,         /* 2nd parameter to sqlite3_bind out of range */
    TF_MDW_SQL_RC_NOTADB,        /* File opened that is not a database file */
    TF_MDW_SQL_RC_NOTICE,        /* Notifications from sqlite3_log() */
    TF_MDW_SQL_RC_WARNING,       /* Warnings from sqlite3_log() */
    TF_MDW_SQL_RC_ROW,           /* sqlite3_step() has another row ready */
    TF_MDW_SQL_RC_DONE,          /* sqlite3_step() has finished executing */
}tf_mdw_sql_errno;

typedef enum
{
    TF_APP_ZK_RC_OK = 0,

    TF_APP_ZK_RC_NET_INIT,
    TF_APP_ZK_RC_NET_NOT_CONNECT,
    TF_APP_ZK_RC_NET_MAP_ALREADY_EXIST,
    TF_APP_ZK_RC_NET_MAP_IS_FULL,

    TF_APP_ZK_RC_PARAM_NULL,
    TF_APP_ZK_RC_PARAM_OUT_OF_RANGE,
    TF_APP_ZK_RC_PARAM_INVALID,
    TF_APP_ZK_RC_PARAM_GET,
    TF_APP_ZK_RC_PARAM_SET,

    TF_APP_ZK_RC_MSG_QUEUE_CREATE,
    TF_APP_ZK_RC_MSG_QUEUE_PUT,
    
    TF_APP_ZK_RC_TASK_CREATE,
    
    TF_APP_ZK_RC_MSG_SYN_VARIFY,
    TF_APP_ZK_RC_MSG_SYN_WAITE,

    TF_APP_ZK_RC_MEM_ALLOCATION,

    TF_APP_ZK_RC_PORT_OPERATION,
    
    TF_APP_ZK_RC_IPC_OPERATION,
    TF_APP_ZK_RC_IPC_PORT_ABSENSE,
    
    TF_APP_ZK_RC_IMPOSSIBLE_ERROR,

}tf_app_zk_errno;

typedef enum
{
    TF_APP_ALARM_RC_OK,

    TF_APP_ALARM_RC_INIT,
    TF_APP_ALARM_RC_PARAM_ONT_OF_RANGE,

}tf_app_alarm_errno;

#endif

#if DEFUNC("错误码转换接口")

typedef enum
{
    OBJ_DRV_SDK = 100,//drv tf obj
    OBJ_DRV_ZK,
    OBJ_DRV_FK,
    
    OBJ_MDW_LIBEV = 300, //middleware net
    OBJ_MDW_SQL,
    OBJ_MDW_NET,

    OBJ_APP_ZK = 400,//app tf obj
    OBJ_APP_ALARM,
    OBJ_APP_STAT,

    OBJ_ID_NUM_OF
    
}TF_OBJ_ID_E;

#define TF_RETURNCODE_BASE                 1000
#define TF_RETURNCODE_MAX_VALUE            (TF_RETURNCODE_BASE*TF_RETURNCODE_BASE*TF_RETURNCODE_BASE)

#define TF_OBJ_DRV_TF_SDK_RC_OFFSET       100000000
#define TF_OBJ_DRV_TF_OLT_RC_OFFSET       101000000
#define TF_OBJ_DRV_TF_NI_RC_OFFSET        102000000
#define TF_OBJ_DRV_TF_ONU_RC_OFFSET       103000000
#define TF_OBJ_DRV_TF_NW_RC_OFFSET        104000000

#define TF_OBJ_MDW_LIBEV_RC_OFFSET         300000000
#define TF_OBJ_MDW_SQLITE_RC_OFFSET        301000000
#define TF_OBJ_MDW_NET_RC_OFFSET           302000000

#define TF_OBJ_APP_TF_ZK_RC_OFFSET    400000000
#define TF_OBJ_APP_TF_TFCFG_RC_OFFSET    401000000
#define TF_OBJ_APP_TF_ONUCFG_RC_OFFSET    402000000
#define TF_OBJ_APP_TF_ONURA_RC_OFFSET     403000000
#define TF_OBJ_APP_TF_PROFILE_RC_OFFSET   404000000
#define TF_OBJ_APP_TF_STAT_RC_OFFSET      405000000
#define TF_OBJ_APP_TF_ALARM_RC_OFFSET     406000000

typedef struct
{
    unsigned int obj_id;
    union 
    {
        unsigned int olt_id;
        unsigned int port_id;
        unsigned int onu_id;
        unsigned int profile_id;
    }sub_obj_id;

    unsigned int err_id;
    
}tf_rc_info;

#define TF_TF_ERR_INFO2RC(rc_info,obj,sub_obj,err_no,rc) do{\
        rc_info.obj_id = obj;\
        *(unsigned int*)&rc_info.sub_obj_id = sub_obj;\
        rc_info.err_id = err_no;\
        tf_info2rc_convert(&rc, &rc_info);\
    }while(0)

#define TF_TF_ERRNO_INFO2RC(rc_info,rc,err_no) do{\
        rc_info.err_id = err_no;tf_info2rc_convert(&rc, &rc_info);\
    }while(0)

unsigned int 
tf_rc2info_convert(unsigned int rc_val, tf_rc_info *p_rc_info);

unsigned int 
tf_info2rc_convert(unsigned int *p_rc_val, tf_rc_info *p_rc_info);


#endif


#endif
