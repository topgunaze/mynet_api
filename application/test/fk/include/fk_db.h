/**************************************************************
* 文件名称:  fk_db.h
* 文件描述:
* 版           本:  V1.00
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>
        keith.gong          2017.02.15      v1.00              Create
**************************************************************/

#ifndef __FK_DB_H__
#define __FK_DB_H__

#include "tf_adaptor_net_if.h"
#include "fk_common.h"
#include "fk_thread_pool.h"


#define ONU_STATUS_NOT_CONFIGURED   0x00000000
#define ONU_STATUS_INACTIVE         0x00000001
#define ONU_STATUS_ACTIVE           0x00000002
#define ONU_STATUS_OMCI_PORT_CPL    0x00000004
#define ONU_STATUS_DELETING         0x00000008

#define ONU_STATUS_PROCESSING       0x40000000
#define ONU_STATUS_CONFIRM          0x80000000

#define MAX_SQLITE_CMD_STR_SIZE     256
#define MAX_SN_STR_SIZE             13
#define MAX_PWD_STR_SIZE            11
#define MAX_LOID_USER_STR_SIZE      25
#define MAX_LOID_CO_STR_SIZE        13

#define DEFAULT_AUTOFIND_AGING_DURATION 300 //unit:sec
#define AUTOFIND_SLENCE_NO_AGING        0
#define SILENCE_AFTER_DELETE            6
#define SILENCE_AUTU_FAILED             30
#define SILENCE_AFTER_CANCEL            6
#define SILENCE_CHECK_DEACTIVE          30
#define SILENCE_AFTER_ACTIVE            1
#define SILENCE_HIT_FILTER              60
#define SILENCE_OMCI_PORT_CFG_FAILED    10
#define SILENCE_ACTIVE_FAILED           10
#define SILENCE_DEACTIVE_FAILED         10
#define SILENCE_PWD_FAILED              10
#define SILENCE_EQD_FAILED              10

typedef struct
{
    uint8_t arr[MAX_LOID_USER_STR_SIZE-1];
}fk_loid_user;

typedef struct
{
    uint8_t arr[MAX_LOID_CO_STR_SIZE-1];
}fk_loid_co;

typedef struct
{
    char vendor[4+1];
    char version[14+1];
    char software_version[14+1];
    char equipment_id[20+1];
    char vendor_product_code[2];
    char omcc_ver;
}fk_onu_attr;

typedef enum
{
    NODE_AUTH_MODE_SN         = 0,
    NODE_AUTH_MODE_SN_PWD,
    NODE_AUTH_MODE_PWD,
    NODE_AUTH_MODE_LOID,
    NODE_AUTH_MODE_LOID_CO,
    NODE_AUTH_MODE_AUTO,
}FK_ONU_AUTH_MODE_E;

typedef enum
{
    FK_ONU_AUTH_TIME_MODE_ALWAYS   = 0,
    FK_ONU_AUTH_TIME_MODE_ONCE_AGING,
    FK_ONU_AUTH_TIME_MODE_ONCE_NO_AGING,
}FK_ONU_AUTH_TIME_MODE_E;

typedef enum
{
    ONU_BLACK_TBL_STMT_IDX_TF_NI,
    ONU_BLACK_TBL_STMT_IDX_SN,
    ONU_BLACK_TBL_STMT_IDX_EXPIRE_AT,
    ONU_BLACK_TBL_STMT_IDX_PHY_UINT,
}ONU_BLACK_TBL_STMT_IDX_E;


typedef enum
{
    ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT,
    ONU_PHY_INFO_TBL_STMT_IDX_STATUS,
    ONU_PHY_INFO_TBL_STMT_IDX_SN,
    ONU_PHY_INFO_TBL_STMT_IDX_PWD,
    ONU_PHY_INFO_TBL_STMT_IDX_LOID_USER,
    ONU_PHY_INFO_TBL_STMT_IDX_LOID_CO,
    ONU_PHY_INFO_TBL_STMT_IDX_EQD,
    ONU_PHY_INFO_TBL_STMT_IDX_ATTR,
    ONU_PHY_INFO_TBL_STMT_IDX_AUTOFIND_TIMESTAMP,
}ONU_PHY_INFO_TBL_STMT_IDX_E;


typedef enum
{
    ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT,
    ONU_AUTH_INFO_TBL_STMT_IDX_PHY_UINT,
    ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE,
    ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_TIME,
    ONU_AUTH_INFO_TBL_STMT_IDX_DEACTIVE,
    ONU_AUTH_INFO_TBL_STMT_IDX_ONLINE_FLAG,
    ONU_AUTH_INFO_TBL_STMT_IDX_ONLINE_TIME,
    ONU_AUTH_INFO_TBL_STMT_IDX_OFFLINE_TIME,
    ONU_AUTH_INFO_TBL_STMT_IDX_DG_TIME,
    ONU_AUTH_INFO_TBL_STMT_IDX_DESC,
    ONU_AUTH_INFO_TBL_STMT_IDX_LINE_PROFILE,
    ONU_AUTH_INFO_TBL_STMT_IDX_SRV_PROFILE,
}ONU_AUTH_INFO_TBL_STMT_IDX_E;


typedef enum
{
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_ONU_UINT,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTH_MODE,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_SN,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_PWD,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT,
    ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG,
}ONU_AUTH_SN_PWD_TBL_STMT_IDX_E;

typedef enum
{
    ONU_AUTH_PWD_TBL_STMT_IDX_ONU_UINT,
    ONU_AUTH_PWD_TBL_STMT_IDX_TF_ID,
    ONU_AUTH_PWD_TBL_STMT_IDX_PWD,
    ONU_AUTH_PWD_TBL_STMT_IDX_TIME_MODE,
    ONU_AUTH_PWD_TBL_STMT_IDX_EXPIRE_AT,
    ONU_AUTH_PWD_TBL_STMT_IDX_AUTHED_FLAG,
}ONU_AUTH_PWD_TBL_STMT_IDX_E;

typedef enum
{
    ONU_AUTH_LOID_TBL_STMT_IDX_ONU_UINT,
    ONU_AUTH_LOID_TBL_STMT_IDX_AUTH_MODE,
    ONU_AUTH_LOID_TBL_STMT_IDX_USER,
    ONU_AUTH_LOID_TBL_STMT_IDX_CO,
    ONU_AUTH_LOID_TBL_STMT_IDX_TIME_MODE,
    ONU_AUTH_LOID_TBL_STMT_IDX_EXPIRE_AT,
    ONU_AUTH_LOID_TBL_STMT_IDX_AUTHED_FLAG,
}ONU_AUTH_LOID_TBL_STMT_IDX_E;

#if 0 //BUILD_TEST

#define FALSE 0
#define TRUE 1

typedef struct
{
    char sn[8];
}
char;

typedef struct
{
    uint8_t arr[10];
}char;

#endif /* BUILD_TEST */


#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif


#define EXIT_ON_SQLITE(db, ret, expect, prompt, new_ret) \
    do \
    { \
        if(ret != expect) \
        { \
            printf("[%s:%d] db_failed %s, %s(rc:%d)\r\n", __FUNCTION__, __LINE__, prompt, sqlite3_errmsg(db), ret); \
            return new_ret; \
        } \
    } \
    while(0);

#define EXIT_ON_SQLITE_UNLOCK(db, ret, expect, prompt, new_ret) \
    do \
    { \
        if(ret != expect) \
        { \
            NODE_DB_UNLOCK; \
            printf("[%s:%d] db_failed %s, %s(rc:%d)\r\n", __FUNCTION__, __LINE__, prompt, sqlite3_errmsg(db), ret); \
            return new_ret; \
        } \
    } \
    while(0);

#define BREAK_ON_SQLITE(db, ret, expect, prompt, new_ret) \
    if(ret != expect) \
    { \
        printf("[%s:%d] db_failed %s, %s(rc:%d)\r\n", __FUNCTION__, __LINE__, prompt, sqlite3_errmsg(db), ret); \
        ret = new_ret; \
        break; \
    }

#define MAX_ONU_FILTER_NUM 64
#define MAX_ONU_AUTOAUTH_RULE_NUM 64

typedef struct
{
    uint8_t     sw_val;
    uint8_t     mode;
    uint32_t    count;
}NODE_BLACKLIST_INFO;


typedef enum
{
    NODE_AUTOAUTH_MODE_ALL,
    NODE_AUTOAUTH_MODE_BY_EQU_ID,
    NODE_AUTOAUTH_MODE_BY_VENDOR_ID,
    NODE_AUTOAUTH_MODE_BY_EQU_ID_SWVER,

    NODE_AUTOAUTH_MODE_NUM,
}NODE_AUTOAUTH_MODE_E;

typedef struct
{
    uint8_t                 sw_val;
    uint8_t                 port_sw_val[MAX_NUM_OF_PORT];
    NODE_AUTOAUTH_MODE_E  mode;
    uint8_t                 target_auth_mode; //0:sn.1:sn-pwd,2:pwd...
    uint8_t                 time_mode; //0:always, 1:once-aging, 2:once-no-aging
    uint32_t                aging_duration;
    uint32_t                count;
}NODE_AUTOAUTH_INFO;


typedef struct
{
    uint8_t                 rule_id;
    char                    equipment_id[20+1];
    char                    vendor[4+1];
    char                    software_version[14+1];
    uint16_t                lineprofile_id;
    uint16_t                srvprofile_id;
    uint32_t                autoauth_num;
}NODE_AUTOAUTH_RULE;


#define COMBINE_DEV_ID_TF_ID(dev_id, port_id) ((port_id) + (dev_id)*MAX_NUM_OF_PORT)


#if DEFUNC("onu_black_tbl")


int
fk_onu_db_onu_black_tbl_should_silence(
                const uint8_t tf_ni,
                const char *p_serial_number);


int
fk_onu_db_onu_black_tbl_add(
                const uint8_t tf_ni,
                const char *p_serial_number,
                const uint32_t *p_phy_uint,     //null: unknown
                const uint32_t sec_duration);    //0: no-aging


int
fk_onu_db_onu_black_tbl_del_by_sn(
                const uint32_t tf_ni,
                const char *p_serial_number);


int
fk_onu_db_onu_black_tbl_del_by_phy_uint(
                const uint32_t phy_uint);


int
fk_onu_db_onu_black_tbl_clear_phy_uint(
                const uint32_t phy_uint);


int
fk_onu_db_onu_black_tbl_update_by_phy_uint(
                const uint32_t phy_uint,
                const uint32_t *p_new_phy_uint, //null:unknown
                const uint32_t sec_duration);


#endif

#if DEFUNC("onu_phy_info_tbl")

int
fk_onu_db_onu_phy_info_tbl_aging_duration_set(
                const uint32_t duration_sec);

int
fk_onu_db_onu_phy_info_tbl_aging_duration_get(
                uint32_t *p_duration_sec);

int
fk_onu_db_onu_phy_info_tbl_get_free_phy_id(
                const uint8_t tf_ni,
                uint16_t *p_phy_id);


int
fk_onu_db_onu_phy_info_tbl_add(
                const char *p_serial_number,
                const uint16_t phy_uint,
                const uint32_t status,
                const uint8_t autofind_flag);


int
fk_onu_db_onu_phy_info_tbl_set_password(
                const uint32_t phy_uint,
                const char *p_password);


int
fk_onu_db_onu_phy_info_tbl_set_loid(
                const uint32_t phy_uint,
                const fk_loid_user *p_loid_user,
                const fk_loid_co *p_loid_co);


int
fk_onu_db_onu_phy_info_tbl_set_eqd(
                const uint32_t phy_uint,
                const uint32_t eqd);


int
fk_onu_db_onu_phy_info_tbl_set_attr(
                const uint32_t phy_uint,
                const fk_onu_attr *p_onu_attr);


int
fk_onu_db_onu_phy_info_tbl_set_status(
                const uint32_t phy_uint,
                const uint32_t status);


int
fk_onu_db_onu_phy_info_tbl_set_autofind(
                const uint32_t phy_uint,
                const uint8_t autofind_flag);


int
fk_onu_db_onu_phy_info_tbl_get_status(
                const uint8_t tf_ni,
                const uint16_t phy_id,
                uint32_t *p_status);


int
fk_onu_db_onu_phy_info_tbl_get_phy_uint(
                const uint8_t tf_ni,
                const char *p_serial_number,
                uint32_t *p_phy_uint,
                uint32_t *p_status);


int
fk_onu_db_onu_phy_info_tbl_get_valid_phy_uint(
                const uint8_t tf_ni,
                const char *p_serial_number,
                uint32_t *p_phy_uint);


int
fk_onu_db_onu_phy_info_tbl_get_first_by_status(
                const uint32_t status,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint/*,
                char *p_serial_number,
                char *p_password,
                fk_loid_user *p_loid_user,
                fk_loid_co *p_loid_co*/);


int
fk_onu_db_onu_phy_info_tbl_get_first_by_pwd(
                const char *p_password,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint,
                char *p_serial_number);


int
fk_onu_db_onu_phy_info_tbl_get_first_by_loid(
                const fk_loid_user *p_loid_user,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint,
                char *p_serial_number);


int
fk_onu_db_onu_phy_info_tbl_get_first_expire(
                uint32_t *p_phy_uint);


int
fk_onu_db_onu_phy_info_tbl_get_first_deleting(
                uint32_t *p_phy_uint);


int
fk_onu_db_onu_phy_info_tbl_get_first_autofind(
                uint32_t *p_phy_uint,
                uint32_t *p_timestamp);


int
fk_onu_db_onu_phy_info_tbl_get_sn_pwd(
                const uint32_t phy_uint,
                char *p_serial_number,
                char *p_password);


int
fk_onu_db_onu_phy_info_tbl_get(
                const uint32_t phy_uint,
                char *p_serial_number,
                char *p_password,
                fk_loid_user *p_loid_user,
                fk_loid_co *p_loid_co,
                uint32_t *p_eqd,
                fk_onu_attr *p_onu_attr);


int
fk_onu_db_onu_phy_info_tbl_del(
                const uint32_t phy_uint);



#endif


#if DEFUNC("onu_auth_info_tbl")

int
fk_onu_db_onu_auth_info_tbl_get_free_onu_id(
                const uint8_t tf_ni,
                uint16_t *p_onu_id);

int
fk_onu_db_onu_auth_info_tbl_free_onu_id(
                const uint8_t tf_ni,
                const uint16_t onu_id);

int
fk_onu_db_onu_auth_info_tbl_add(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                const uint8_t auth_mode,
                const uint32_t lineprofile_id,
                const uint32_t srvprofile_id);


int
fk_onu_db_onu_auth_info_tbl_auth_mode_get(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                uint8_t *p_auth_mode);


int
fk_onu_db_onu_auth_info_tbl_get_first(
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode,
                uint8_t *p_deactive);

int
fk_onu_db_onu_auth_info_tbl_set_online(
                const uint32_t onu_uint,
                const uint32_t phy_uint);

int
fk_onu_db_onu_auth_info_tbl_set_offline(
                const uint32_t onu_uint,
                uint8_t *p_aging_flag);


int
fk_onu_db_onu_auth_info_tbl_set_offline_by_phy_uint(
                const uint32_t phy_uint,
                uint8_t *p_aging_flag);


int
fk_onu_db_onu_auth_info_tbl_set_dg_time(
                const uint32_t onu_uint);


int
fk_onu_db_onu_auth_info_tbl_set_desc(
                const uint32_t onu_uint,
                const char *p_desc); //null: clear


int
fk_onu_db_onu_auth_info_tbl_set_deactive_flag(
                const uint32_t onu_uint,
                const uint8_t deactive);


int
fk_onu_db_onu_auth_info_tbl_get_phy_uint(
                const uint32_t onu_uint,
                uint32_t *p_phy_uint);


int
fk_onu_db_onu_auth_info_tbl_get_onu_uint(
                const uint32_t phy_uint,
                uint32_t *p_onu_uint);

int
fk_onu_db_onu_auth_info_tbl_get_deactive_flag(
                const uint32_t onu_uint,
                uint8_t *p_deactive,
                uint8_t *p_auth_mode);

int
fk_onu_db_onu_auth_info_tbl_get_profile(
                const uint32_t onu_uint,
                uint32_t *p_lineprofile_id,
                uint32_t *p_srvprofile_id);


int
fk_onu_db_onu_auth_info_tbl_get_line_profile_counter(
                const uint32_t lineprofile_id,
                uint16_t *p_bind_counter);

int
fk_onu_db_onu_auth_info_tbl_get_srv_profile_counter(
                const uint32_t srvprofile_id,
                uint16_t *p_bind_counter);


int
fk_onu_db_onu_auth_info_tbl_get_first_deactive(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);


int
fk_onu_db_onu_auth_info_tbl_get_first_active(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);


int
fk_onu_db_onu_auth_info_tbl_get_first_online(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint32_t *p_phy_uint);


int
fk_onu_db_onu_auth_info_tbl_get_by_desc(
                const char *p_desc,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);


int
fk_onu_db_onu_auth_info_tbl_get(
                const uint32_t onu_uint,
                uint32_t *p_phy_uint,
                uint8_t *p_online_flag,
                uint8_t *p_deactive_flag,
                uint8_t *p_auth_mode);

int
fk_onu_db_onu_auth_info_tbl_del(
                const uint32_t onu_uint);


#endif


#if DEFUNC("onu_auth_sn_pwd_tbl")


int
fk_onu_db_onu_auth_sn_pwd_tbl_add(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                const FK_ONU_AUTH_MODE_E auth_mode,
                const char *p_serial_number,
                const char *p_password, //can be null
                const FK_ONU_AUTH_TIME_MODE_E time_mode,
                const uint32_t sec_duration,
                const uint32_t lineprofile_id,
                const uint32_t srvprofile_id);


int
fk_onu_db_onu_auth_sn_pwd_tbl_check(
                const char *p_serial_number,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);

int
fk_onu_db_onu_auth_sn_pwd_tbl_set_authed(
                const uint32_t onu_uint);

int
fk_onu_db_onu_auth_sn_pwd_tbl_get_sn_pwd(
                const uint32_t onu_uint,
                uint8_t *p_auth_mode,
                char *p_serial_number,
                char *p_password);


int
fk_onu_db_onu_auth_sn_pwd_tbl_get_onu_uint_by_sn(
                const char *p_serial_number,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);


int
fk_onu_db_onu_auth_sn_pwd_tbl_get_onu_uint_by_pwd_first(
                const uint32_t onu_uint_start,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);

#endif

#if DEFUNC("onu_auth_pwd_tbl")


int
fk_onu_db_onu_auth_pwd_tbl_add(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                const char *p_password,
                const FK_ONU_AUTH_TIME_MODE_E time_mode,
                const uint32_t sec_duration,
                const uint32_t lineprofile_id,
                const uint32_t srvprofile_id);


int
fk_onu_db_onu_auth_pwd_tbl_check(
                const uint8_t tf_ni,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);

int
fk_onu_db_onu_auth_pwd_tbl_set_authed(
                const uint32_t onu_uint);

int
fk_onu_db_onu_auth_pwd_tbl_get_pwd(
                const uint32_t onu_uint,
                uint8_t *p_auth_mode,
                char *p_password);


int
fk_onu_db_onu_auth_pwd_tbl_get_onu_uint_by_pwd_first(
                const uint32_t onu_uint_start,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);

#endif

#if DEFUNC("onu_auth_loid_tbl")

int
fk_onu_db_onu_auth_loid_tbl_add(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                const FK_ONU_AUTH_MODE_E auth_mode,
                const fk_loid_user *p_loid_user,
                const fk_loid_co *p_loid_co, //can be null
                const FK_ONU_AUTH_TIME_MODE_E time_mode,
                const uint32_t sec_duration,
                const uint32_t lineprofile_id,
                const uint32_t srvprofile_id);


int
fk_onu_db_onu_auth_loid_tbl_check(
                const fk_loid_user *p_loid_user,
                const fk_loid_co *p_loid_co,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);

int
fk_onu_db_onu_auth_loid_tbl_set_authed(
                const uint32_t onu_uint);

int
fk_onu_db_onu_auth_loid_tbl_get_loid(
                const uint32_t onu_uint,
                uint8_t *p_auth_mode,
                fk_loid_user *p_loid_user,
                fk_loid_co *p_loid_co);


int
fk_onu_db_onu_auth_loid_tbl_get_onu_uint(
                const fk_loid_user *p_loid_user,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode);


#endif


#if DEFUNC("onu_filter_tbl")

int
fk_onu_db_onu_fliter_tbl_set_switch(
                uint8_t sw_val);

int
fk_onu_db_onu_fliter_tbl_get_switch(
                uint8_t *p_sw);

int
fk_onu_db_onu_fliter_tbl_set_mode(
                uint8_t mode);

int
fk_onu_db_onu_fliter_tbl_get_mode(
                uint8_t *p_mode);

int
fk_onu_db_onu_fliter_tbl_get_records_count(
                uint32_t *p_count);

int
fk_onu_db_onu_fliter_tbl_add(
                const uint8_t mask_len,
                const char *p_sn_str);

int
fk_onu_db_onu_filter_tbl_del(
                const uint16_t offset);

int
fk_onu_db_onu_filter_tbl_del_all(void);

//search the sn in db
int
fk_onu_db_onu_filter_tbl_sn_hit_check(
                const char *p_serial_number);

int
fk_onu_db_onu_filter_tbl_get_first(
                const int start_idx,
                int *p_idx,
                char *p_sn_str,
                uint8_t *p_sn_mask_len,
                uint32_t*p_hit_count);

#endif



#if DEFUNC("autoauth_rule_tbl")

int
fk_onu_db_autoauth_rule_tbl_set_global_switch(
                uint8_t sw_val);

int
fk_onu_db_autoauth_rule_tbl_get_global_switch(
                uint8_t *p_sw);

int
fk_onu_db_autoauth_rule_tbl_set_switch(
                const uint16_t dev_id,
                const uint16_t port_id,
                const uint8_t  sw_val);

int
fk_onu_db_autoauth_rule_tbl_get_switch(
                uint16_t dev_id,
                uint16_t port_id,
                uint8_t* p_sw);


int
fk_onu_db_autoauth_rule_tbl_get_mode(
                uint8_t *p_mode,
                uint8_t *p_target_auth_mode,
                uint8_t *p_time_mode,
                uint32_t *p_agint_duration);

int
fk_onu_db_autoauth_rule_tbl_get_rule_count(
                uint32_t* p_count);


int
fk_onu_db_autoauth_rule_tbl_reset_mode(
                const uint8_t mode,
                const uint8_t target_auth_mode,
                const uint8_t time_mode,
                const uint32_t aging_duration);


int
fk_onu_db_autoauth_rule_tbl_add(
                const uint16_t lineprofile_id,
                const uint16_t srvprofile_id,
                const char *p_vendor,
                const char *p_equipment_id,
                const char *p_sw_version);


int
fk_onu_db_autoauth_rule_tbl_check(
                const char * p_vendor,
                const char * p_equipment_id,
                const char * p_sw_version,
                uint16_t *p_lineprofile_id,
                uint16_t *p_srvprofile_id,
                uint8_t *p_target_auth_mode);


int
fk_onu_db_autoauth_rule_tbl_get_first(
                const uint32_t rule_id_start,
                int *p_rule_id,
                uint16_t *p_lineprofile_id,
                uint16_t *p_srvprofile_id,
                uint32_t *p_autoauth_num,
                char *p_vendor,
                char *p_equipment_id,
                char *p_sw_version);

int
fk_onu_db_autoauth_rule_tbl_del(
                const uint32_t rule_id_offset); //start from 0

int
fk_onu_db_autoauth_rule_tbl_del_all(void);


#endif


#if DEFUNC("外部接口函数")


int
fk_db_init(void);

int
fk_onu_db_uninit(void);


#endif


#endif
