/**************************************************************
* 文件名称: 
* 文件描述:
* 版           本:  
* 修改历史:
*     <修改者>     <时间>      <版本 >     <描述>
**************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "sqlite3.h"
#include "fk_init.h"


static sqlite3 *p_fk_onu_db = NULL;

static uint32_t autofind_aging_duration;

static NODE_BLACKLIST_INFO g_onu_filter_info;

static uint8_t onu_uint_bit_map[MAX_NUM_OF_PORT][MAX_NODE_EACH_PORT/8];

static sem_t fk_onu_db_sem;

#define NODE_DB_LOCK  do{sem_wait(&fk_onu_db_sem);}while(0)
#define NODE_DB_UNLOCK  do{sem_post(&fk_onu_db_sem);}while(0)


static int
fk_onu_db_onu_auth_sn_pwd_tbl_aging(
                const uint32_t onu_uint,
                uint8_t *p_aging_flag);

#if DEFUNC("内部处理函数")


static int
fk_sn_to_str(
                const char *p_serial_number,
                char *p_sn_str,
                const int sn_str_size)
{
    if(!p_serial_number || !p_sn_str || sn_str_size <= 12) //TODO keith.gong macro
        return -1;

    return 0;
}


static int
fk_str_to_sn(
                const unsigned char *p_sn_str,
                char *p_serial_number)
{
    uint32_t idx;
    const unsigned char *p_str;
    int hex;

    if(!p_sn_str || !p_serial_number)
        return -1;

    for(idx = 0, p_str = p_sn_str + 4; idx < 4; idx++, p_str += 2)
    {
        if(sscanf((const char*)p_str, "%02x", &hex) <= 0)
            return -1;

    }

    return 0;
}


static int
fk_pwd_to_str(
                const char *p_password,
                char *pwd_str,
                const int pwd_str_size)
{
    if(!p_password || !pwd_str || pwd_str_size <= sizeof(char))
        return -1;


    return 0;
}

static int
fk_str_to_pwd(
                const unsigned char *pwd_str,
                char *p_password)
{
    if(!p_password)
        return -1;

    return 0;
}


static int
fk_str_to_loid_user(
                const unsigned char *p_loid_user_str,
                fk_loid_user *p_loid_user)
{
    if(!p_loid_user)
        return -1;

    if(!p_loid_user_str)
        memset((char *)p_loid_user->arr, 0, sizeof(fk_loid_user));
    else
        strncpy((char *)p_loid_user->arr, (const char*)p_loid_user_str, sizeof(fk_loid_user));

    return 0;
}


static int
fk_loid_user_to_str(
                const fk_loid_user *p_loid_user,
                char *p_loid_user_str,
                const uint32_t loid_user_str_size)
{
    if(!p_loid_user || !p_loid_user_str || loid_user_str_size <= sizeof(fk_loid_user))
        return -1;

    snprintf(p_loid_user_str, loid_user_str_size, "%s", (const char *)p_loid_user->arr);

    return 0;
}


static int
fk_str_to_loid_co(
                const unsigned char *p_loid_co_str,
                fk_loid_co *p_loid_co)
{
    if(!p_loid_co)
        return -1;

    if(!p_loid_co_str)
        memset((char *)p_loid_co->arr, 0, sizeof(fk_loid_co));
    else
        strncpy((char *)p_loid_co->arr, (const char*)p_loid_co_str, sizeof(fk_loid_co));

    return 0;
}


static int
fk_loid_co_to_str(
                const fk_loid_co *p_loid_co,
                char *p_loid_co_str,
                const uint32_t loid_co_str_size)
{
    if(!p_loid_co || !p_loid_co_str || loid_co_str_size <= sizeof(fk_loid_co))
        return -1;

    snprintf(p_loid_co_str, loid_co_str_size, "%s", (const char *)p_loid_co->arr);

    return 0;
}


static void
fk_onu_uint_bit_map_clear(
                const uint8_t port_id,
                const uint16_t onu_id)
{
    onu_uint_bit_map[port_id][onu_id/8] &= ~(1U << (onu_id%8));
}


static int
fk_onu_uint_bit_map_alloc(
                const uint8_t port_id,
                uint16_t *p_onu_id)
{
    uint32_t idx1, idx2;
    static const uint8_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    uint8_t val;

    for (idx1 = 0; idx1 < MAX_NODE_EACH_PORT/8; idx1++)
    {
        if (onu_uint_bit_map[port_id][idx1] == 0xFFU)
            continue;

        val = ~onu_uint_bit_map[port_id][idx1];
        for (idx2 = 0; idx2 < 8; idx2++)
        {
            if(val & mask[idx2])
            {
                *p_onu_id = idx1*8+idx2;
                onu_uint_bit_map[port_id][idx1] |= mask[idx2];
                return SQLITE_OK;
            }
        }
    }

    return SQLITE_PERM;
}


#endif

#if DEFUNC("onu_black_tbl")


static int
fk_onu_db_onu_black_tbl_aging(
                const long timestamp_now)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    static long last_aging_timestamp;

    /* 1s 内只执行一次老化， 减少对数据库的读写*/
    if(last_aging_timestamp == timestamp_now)
        return SQLITE_OK;

    last_aging_timestamp = timestamp_now;

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_black_tbl where expire_at <= %ld and expire_at not null", timestamp_now);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_black_tbl_should_silence(
                const uint8_t tf_ni,
                const char *p_serial_number)
{
    int ret;
    struct sysinfo sys_info;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char sn_str[MAX_SN_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_serial_number)
        return FALSE;

    sysinfo(&sys_info);
    fk_onu_db_onu_black_tbl_aging(sys_info.uptime);

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));
    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_black_tbl where tf_ni = %d and sn like \'%s\'", tf_ni, sn_str);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, FALSE);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return TRUE;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return FALSE;
}


int
fk_onu_db_onu_black_tbl_add(
                const uint8_t tf_ni,
                const char *p_serial_number,
                const uint32_t *p_phy_uint,     //null: unknown
                const uint32_t sec_duration)    //0: no-aging
{
    int ret;
    sqlite3_stmt *stmt;
    const char* sql = "insert or replace into onu_black_tbl values(?, ?, ?, ?)"; //warnning: override the old record
    struct sysinfo sys_info;
    char sn_str[MAX_SN_STR_SIZE];

    if(!p_serial_number)
        return SQLITE_PERM;

    sysinfo(&sys_info);

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, sql, strlen(sql), &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 1, tf_ni);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_text(stmt, 2, sn_str, sizeof(sn_str), NULL);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        if(sec_duration)
            ret = sqlite3_bind_int(stmt, 3, sys_info.uptime + sec_duration);
        else
            ret = sqlite3_bind_null(stmt, 3);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        if(p_phy_uint)
            ret = sqlite3_bind_int(stmt, 4, *p_phy_uint);
        else
            ret = sqlite3_bind_null(stmt, 4);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, sql, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;
    }
    while(0);

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_black_tbl_del_by_sn(
                const uint32_t tf_ni,
                const char *p_serial_number)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char sn_str[MAX_SN_STR_SIZE];

    if(!p_serial_number)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));
    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_black_tbl where sn like \'%s\' and tf_ni = %d", sn_str, tf_ni);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_black_tbl_del_by_phy_uint(
                const uint32_t phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_black_tbl where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_black_tbl_clear_phy_uint(
                const uint32_t phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_black_tbl set phy_uint = null where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_black_tbl_update_by_phy_uint(
                const uint32_t phy_uint,
                const uint32_t *p_new_phy_uint, //null:unknown
                const uint32_t sec_duration)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    struct sysinfo sys_info;

    if(!sec_duration)
        return SQLITE_PERM;

    sysinfo(&sys_info);

    if(p_new_phy_uint)
        snprintf(cmd_str, sizeof(cmd_str),
            "update onu_black_tbl set expire_at = %lu, phy_uint = %d where phy_uint = %d", sys_info.uptime + sec_duration, *p_new_phy_uint, phy_uint);
    else
        snprintf(cmd_str, sizeof(cmd_str),
            "update onu_black_tbl set expire_at = %lu, phy_uint = null where phy_uint = %d", sys_info.uptime + sec_duration, phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


static int
fk_create_onu_black_tbl(void)
{
    int ret;

    ret = sqlite3_exec(p_fk_onu_db, "drop table if exists onu_black_tbl", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "drop onu_black_tbl failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "create table onu_black_tbl("
                                            "tf_ni int8 not null, "
                                            "sn text not null COLLATE NOCASE, "
                                            "expire_at int,"        // may be null
                                            "phy_uint int unique,"  // may be null
                                            "constraint auth_key primary key(tf_ni, sn),"
                                            "FOREIGN KEY(phy_uint) REFERENCES onu_phy_info_tbl(phy_uint))",
                                            NULL, NULL, NULL);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "create onu_black_tbl failed", ret);

    return SQLITE_OK;
}


#endif

#if DEFUNC("onu_phy_info_tbl")

int
fk_onu_db_onu_phy_info_tbl_aging_duration_set(
                const uint32_t duration_sec)
{
    autofind_aging_duration = duration_sec;

    return 0;
}

int
fk_onu_db_onu_phy_info_tbl_aging_duration_get(
                uint32_t *p_duration_sec)
{
    *p_duration_sec = autofind_aging_duration;

    return 0;
}

int
fk_onu_db_onu_phy_info_tbl_get_free_phy_id(
                const uint8_t tf_ni,
                uint16_t *p_phy_id)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    uint32_t free_phy_uint;
    sqlite3_stmt *stmt = NULL;
    const uint32_t phy_uint_start = PHY_ID_2_PHY_UINT(tf_ni, 0);
    const uint32_t phy_uint_end   = PHY_ID_2_PHY_UINT(tf_ni, MAX_NODE_EACH_PORT-1); /*TODO: MAX_NODE_EACH_PORT GTF ?*/

    if(tf_ni >= MAX_NUM_OF_PORT || !p_phy_id)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and phy_uint <= %d order by phy_uint",
        phy_uint_start, phy_uint_end);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    free_phy_uint = phy_uint_start;
    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (free_phy_uint != sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT))
        {
            *p_phy_id = PHY_UINT_2_PHY_ID(free_phy_uint);
            ret = SQLITE_OK;
            break;
        }

        free_phy_uint += 1;
    }

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    if(ret && free_phy_uint <= phy_uint_end)
    {
        *p_phy_id = PHY_UINT_2_PHY_ID(free_phy_uint);
        ret = SQLITE_OK;
    }

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_add(
                const char *p_serial_number,
                const uint16_t phy_uint,
                const uint32_t status,
                const uint8_t autofind_flag)
{
    int ret;
    sqlite3_stmt *stmt;
    const char *sql = "insert into onu_phy_info_tbl values(?, ?, ?, NULL, NULL, NULL, 0, NULL, ?)";
    char sn_str[MAX_SN_STR_SIZE];
    struct sysinfo sys_info;

    if(!p_serial_number || !VALID_PHY_UINT(phy_uint))
        return SQLITE_PERM;

    sysinfo(&sys_info);

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, sql, strlen(sql), &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 1, phy_uint);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 2, status);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_text(stmt, 3, sn_str, sizeof(sn_str), NULL);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        if(autofind_flag)
            ret = sqlite3_bind_int(stmt, 4, sys_info.uptime);
        else
            ret = sqlite3_bind_null(stmt, 4);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, sql, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;
    }
    while(0);

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_set_password(
                const uint32_t phy_uint,
                const char *p_password)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char pwd_str[MAX_PWD_STR_SIZE];

    if(!p_password)
        return SQLITE_PERM;

    fk_pwd_to_str(p_password, pwd_str, MAX_PWD_STR_SIZE);
    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_phy_info_tbl set password = \'%s\' where phy_uint = %d", pwd_str, phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_phy_info_tbl_set_loid(
                const uint32_t phy_uint,
                const fk_loid_user *p_loid_user,
                const fk_loid_co *p_loid_co)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char loid_user_str[MAX_LOID_USER_STR_SIZE];
    char loid_co_str[MAX_LOID_CO_STR_SIZE];

    if(!p_loid_user || !p_loid_co)
        return SQLITE_PERM;

    fk_loid_user_to_str(p_loid_user, loid_user_str, MAX_LOID_USER_STR_SIZE);
    fk_loid_co_to_str(p_loid_co, loid_co_str, MAX_LOID_CO_STR_SIZE);
    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_phy_info_tbl set loid_user = \'%s\', loid_co = \'%s\' where phy_uint = %d", loid_user_str, loid_co_str, phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_phy_info_tbl_set_eqd(
                const uint32_t phy_uint,
                const uint32_t eqd)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_phy_info_tbl set eqd = %d where phy_uint = %d", eqd, phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_phy_info_tbl_set_attr(
                const uint32_t phy_uint,
                const fk_onu_attr *p_onu_attr)
{
    int ret;
    sqlite3_stmt *stmt;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    int len;

    if(!p_onu_attr)
        return SQLITE_PERM;

    len = snprintf(cmd_str, sizeof(cmd_str),
        "update onu_phy_info_tbl set attr = ? where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);

    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, len, &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        ret = sqlite3_bind_blob(stmt, 1, p_onu_attr, sizeof(fk_onu_attr), NULL);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, cmd_str, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;
    }
    while(0);

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_set_status(
                const uint32_t phy_uint,
                const uint32_t status)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_phy_info_tbl set status = %d where phy_uint = %d", status, phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_phy_info_tbl_set_autofind(
                const uint32_t phy_uint,
                const uint8_t autofind_flag)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    struct sysinfo sys_info;

    sysinfo(&sys_info);

    if(autofind_flag)
        snprintf(cmd_str, sizeof(cmd_str),
            "update onu_phy_info_tbl set autofind_timestamp = %d where phy_uint = %d", (uint32_t)sys_info.uptime, phy_uint);
    else
        snprintf(cmd_str, sizeof(cmd_str),
            "update onu_phy_info_tbl set autofind_timestamp = null where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_status(
                const uint8_t tf_ni,
                const uint16_t phy_id,
                uint32_t *p_status)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_status)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint = %d", PHY_ID_2_PHY_UINT(tf_ni, phy_id));

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_status = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_STATUS);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_phy_uint(
                const uint8_t tf_ni,
                const char *p_serial_number,
                uint32_t *p_phy_uint,
                uint32_t *p_status)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    char sn_str[MAX_SN_STR_SIZE];
    const uint32_t phy_uint_start = PHY_ID_2_PHY_UINT(tf_ni, 0);
    const uint32_t phy_uint_end   = PHY_ID_2_PHY_UINT(tf_ni, MAX_NODE_EACH_PORT-1); /*TODO: MAX_NODE_EACH_PORT GTF ?*/

    if(!p_serial_number || !p_phy_uint || !p_status)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where sn like \'%s\' and phy_uint >= %d and phy_uint <= %d order by phy_uint", sn_str, phy_uint_start, phy_uint_end);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        *p_status = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_STATUS);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_valid_phy_uint(
                const uint8_t tf_ni,
                const char *p_serial_number,
                uint32_t *p_phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    char sn_str[MAX_SN_STR_SIZE];
    const uint32_t phy_uint_start = PHY_ID_2_PHY_UINT(tf_ni, 0);
    const uint32_t phy_uint_end   = PHY_ID_2_PHY_UINT(tf_ni, MAX_NODE_EACH_PORT-1); /*TODO: MAX_NODE_EACH_PORT GTF ?*/

    if(!p_serial_number || !p_phy_uint)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where sn like \'%s\' and phy_uint >= %d and phy_uint <= %d and status != %d order by phy_uint", sn_str, phy_uint_start, phy_uint_end, ONU_STATUS_DELETING);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_by_status(
                const uint32_t status,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint/*,
                char *p_serial_number,
                char *p_password,
                fk_loid_user *p_loid_user,
                fk_loid_co *p_loid_co*/)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_phy_uint/* || !p_serial_number || !p_password*/)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and phy_uint <= %d and status = %d order by phy_uint", *p_phy_uint, phy_uint_end, status);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        //fk_str_to_sn(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_SN), p_serial_number);
        //fk_str_to_pwd(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PWD), p_password);
        //fk_str_to_loid_user(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_LOID_USER), p_loid_user);
        //fk_str_to_loid_co(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_LOID_CO), p_loid_co);
        sqlite3_finalize(stmt);

        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);

    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_by_pwd(
                const char *p_password,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint,
                char *p_serial_number)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    char pwd_str[MAX_PWD_STR_SIZE];

    if(!p_password || !p_phy_uint || !p_serial_number)
        return SQLITE_PERM;

    fk_pwd_to_str(p_password, pwd_str, MAX_PWD_STR_SIZE);
    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and phy_uint <= %d and password glob \'%s\' order by phy_uint", *p_phy_uint, phy_uint_end, pwd_str);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        fk_str_to_sn(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_SN), p_serial_number);
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_by_loid(
                const fk_loid_user *p_loid_user,
                const uint32_t phy_uint_end,
                uint32_t *p_phy_uint,
                char *p_serial_number)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    char loid_user_str[MAX_LOID_USER_STR_SIZE];

    if(!p_loid_user || !p_phy_uint || !p_serial_number)
        return SQLITE_PERM;

    fk_loid_user_to_str(p_loid_user, loid_user_str, MAX_LOID_USER_STR_SIZE);
    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and phy_uint <= %d and loid_user glob \'%s\' order by phy_uint", *p_phy_uint, phy_uint_end, loid_user_str);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        fk_str_to_sn(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_SN), p_serial_number);
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_expire(
                uint32_t *p_phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    struct sysinfo sys_info;

    if(!p_phy_uint)
        return SQLITE_PERM;

    sysinfo(&sys_info);

    //no aging
    if(!autofind_aging_duration)
        return SQLITE_NOTFOUND;

    //keith.gong, prevent overflow
    if(sys_info.uptime <= autofind_aging_duration)
        return SQLITE_NOTFOUND;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and autofind_timestamp <= %d", *p_phy_uint, (uint32_t)sys_info.uptime - autofind_aging_duration);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, 0);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_deleting(
                uint32_t *p_phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_phy_uint)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and status = %d", *p_phy_uint, ONU_STATUS_DELETING);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, 0);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_first_autofind(
                uint32_t *p_phy_uint,
                uint32_t *p_timestamp)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    struct sysinfo sys_info;

    if(!p_phy_uint || !p_timestamp)
        return SQLITE_PERM;

    sysinfo(&sys_info);

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint >= %d and autofind_timestamp not null order by phy_uint", *p_phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PHY_UINT);
        *p_timestamp = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_AUTOFIND_TIMESTAMP);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_get_sn_pwd(
                const uint32_t phy_uint,
                char *p_serial_number,
                char *p_password)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_serial_number)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        fk_str_to_sn(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_SN), p_serial_number);
        fk_str_to_pwd(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PWD), p_password);

        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_phy_info_tbl_get(
                const uint32_t phy_uint,
                char *p_serial_number,
                char *p_password,
                fk_loid_user *p_loid_user,
                fk_loid_co *p_loid_co,
                uint32_t *p_eqd,
                fk_onu_attr *p_onu_attr)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    int blob_size;

    if(!p_serial_number || !p_password || !p_loid_user || !p_loid_co || !p_eqd || !p_onu_attr)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_phy_info_tbl where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        fk_str_to_sn(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_SN), p_serial_number);
        fk_str_to_pwd(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_PWD), p_password);
        fk_str_to_loid_user(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_LOID_USER), p_loid_user);
        fk_str_to_loid_co(sqlite3_column_text(stmt, ONU_PHY_INFO_TBL_STMT_IDX_LOID_CO), p_loid_co);
        *p_eqd = sqlite3_column_int(stmt, ONU_PHY_INFO_TBL_STMT_IDX_EQD);

        blob_size = sqlite3_column_bytes(stmt, ONU_PHY_INFO_TBL_STMT_IDX_ATTR);
        if(blob_size == sizeof(fk_onu_attr))
            memcpy(p_onu_attr, sqlite3_column_blob(stmt, ONU_PHY_INFO_TBL_STMT_IDX_ATTR), sizeof(fk_onu_attr));

        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_phy_info_tbl_del(
                const uint32_t phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_phy_info_tbl where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


static int
fk_create_onu_phy_info_tbl(void)
{
    int ret;
    char * p_errmsg = NULL;

    ret = sqlite3_exec(p_fk_onu_db, "drop table if exists onu_phy_info_tbl", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "drop onu_phy_info_tbl failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "create table "
                                            "onu_phy_info_tbl("
                                                "phy_uint int unique, "
                                                "status int,"
                                                "sn text not null COLLATE NOCASE, "
                                                "password text,"
                                                "loid_user text,"
                                                "loid_co text,"
                                                "eqd int8, "
                                                "attr blob,"
                                                "autofind_timestamp int, "
                                                "constraint auth_key primary key(phy_uint, sn))",
                                            NULL, NULL, &p_errmsg );
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "create onu_phy_info_tbl failed", ret);

    return SQLITE_OK;
}


#endif


#if DEFUNC("onu_auth_info_tbl")

static int
fk_does_sn_offline(
                const uint8_t tf_ni,
                const char *p_serial_number)
{
    int ret;
    uint32_t phy_uint, onu_uint;

    ret = fk_onu_db_onu_phy_info_tbl_get_valid_phy_uint(tf_ni, p_serial_number, &phy_uint);
    if(ret)
        return TRUE;

    ret = fk_onu_db_onu_auth_info_tbl_get_onu_uint(phy_uint, &onu_uint);
    if(ret)
        return TRUE;

    return FALSE;
}


int
fk_onu_db_onu_auth_info_tbl_get_free_onu_id(
                const uint8_t tf_ni,
                uint16_t *p_onu_id)
{

    int ret;

    NODE_DB_LOCK;

    ret = fk_onu_uint_bit_map_alloc(tf_ni, p_onu_id);

    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_free_onu_id(
                const uint8_t tf_ni,
                const uint16_t onu_id)
{
    NODE_DB_LOCK;

    fk_onu_uint_bit_map_clear(tf_ni, onu_id);

    NODE_DB_UNLOCK;

    return 0;
}


int
fk_onu_db_onu_auth_info_tbl_add(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                const uint8_t auth_mode,
                const uint32_t lineprofile_id,
                const uint32_t srvprofile_id)
{
    int ret;
    sqlite3_stmt *stmt;
    const char *sql = "insert into onu_auth_info_tbl values(?, NULL, ?, ?, 0, 0, null, null, null, null, ?, ?)";
    uint32_t onu_uint;
    struct sysinfo sys_info;

    onu_uint = NODE_ID_2_NODE_UINT(tf_ni, onu_id);
    if(!VALID_NODE_UINT(onu_uint))
        return SQLITE_PERM;

    NODE_DB_LOCK;

    sysinfo(&sys_info);

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, sql, strlen(sql), &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 1, onu_uint);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 2, auth_mode);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 3, sys_info.uptime);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 4, lineprofile_id);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 5, srvprofile_id);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, sql, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;
    }
    while(0);

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_auth_mode_get(
                const uint8_t tf_ni,
                const uint16_t onu_id,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint = %d", NODE_ID_2_NODE_UINT(tf_ni, onu_id));

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_first(
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode,
                uint8_t *p_deactive)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_onu_uint || !p_auth_mode || !p_deactive)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint >= %d order by onu_uint", *p_onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        *p_deactive = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_DEACTIVE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}

int
fk_onu_db_onu_auth_info_tbl_set_online(
                const uint32_t onu_uint,
                const uint32_t phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    struct sysinfo sys_info;

    sysinfo(&sys_info);

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_auth_info_tbl set online_time = %d, phy_uint = %d, online_flag = 1 where onu_uint = %d", (uint32_t)sys_info.uptime, phy_uint, onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}

int
fk_onu_db_onu_auth_info_tbl_set_offline(
                const uint32_t onu_uint,
                uint8_t *p_aging_flag)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    struct sysinfo sys_info;

    sysinfo(&sys_info);

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_auth_info_tbl set offline_time = %d, online_flag = 0 where onu_uint = %d", (uint32_t)sys_info.uptime, onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    /* check aging */
    *p_aging_flag = 0;
    fk_onu_db_onu_auth_sn_pwd_tbl_aging(onu_uint, p_aging_flag);

    return SQLITE_OK;
}


int
fk_onu_db_onu_auth_info_tbl_set_offline_by_phy_uint(
                const uint32_t phy_uint,
                uint8_t *p_aging_flag)
{
    int ret;
    uint32_t onu_uint;

    *p_aging_flag = 0;

    ret = fk_onu_db_onu_auth_info_tbl_get_onu_uint(phy_uint, &onu_uint);
    if(ret)
        return ret;

    return fk_onu_db_onu_auth_info_tbl_set_offline(onu_uint, p_aging_flag);
}


int
fk_onu_db_onu_auth_info_tbl_set_dg_time(
                const uint32_t onu_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    struct sysinfo sys_info;

    sysinfo(&sys_info);

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_auth_info_tbl set dg_time = %d where onu_uint = %d", (uint32_t)sys_info.uptime, onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_auth_info_tbl_set_desc(
                const uint32_t onu_uint,
                const char *p_desc) //null: clear
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    if(p_desc)
        snprintf(cmd_str, sizeof(cmd_str),
                "update onu_auth_info_tbl set desc = \'%s\' where onu_uint = %d", p_desc, onu_uint);
    else
        snprintf(cmd_str, sizeof(cmd_str),
                "update onu_auth_info_tbl set desc = null where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_auth_info_tbl_set_deactive_flag(
                const uint32_t onu_uint,
                const uint8_t deactive)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
            "update onu_auth_info_tbl set deactive = %d where onu_uint = %d", deactive ? 1 : 0, onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}


int
fk_onu_db_onu_auth_info_tbl_get_phy_uint(
                const uint32_t onu_uint,
                uint32_t *p_phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint = %d and phy_uint not null", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_PHY_UINT);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_onu_uint(
                const uint32_t phy_uint,
                uint32_t *p_onu_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_onu_uint)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where phy_uint = %d", phy_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}

int
fk_onu_db_onu_auth_info_tbl_get_deactive_flag(
                const uint32_t onu_uint,
                uint8_t *p_deactive,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_deactive || !p_auth_mode)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_deactive = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_DEACTIVE);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}

int
fk_onu_db_onu_auth_info_tbl_get_profile(
                const uint32_t onu_uint,
                uint32_t *p_lineprofile_id,
                uint32_t *p_srvprofile_id)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_lineprofile_id = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_LINE_PROFILE);
        *p_srvprofile_id = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_SRV_PROFILE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_line_profile_counter(
                const uint32_t lineprofile_id,
                uint16_t *p_bind_counter)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where lineprofile_id = %d", lineprofile_id);

    *p_bind_counter = 0;

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
        *p_bind_counter += 1;

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_srv_profile_counter(
                const uint32_t srvprofile_id,
                uint16_t *p_bind_counter)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where srvprofile_id = %d", srvprofile_id);

    *p_bind_counter = 0;

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
        *p_bind_counter += 1;

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_first_deactive(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint >= %d and onu_uint <= %d and deactive = 1 order by onu_uint", *p_onu_uint, onu_uint_end);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_first_active(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint >= %d and onu_uint <= %d and deactive = 0 order by onu_uint", *p_onu_uint, onu_uint_end);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_first_online(
                const uint32_t onu_uint_end,
                uint32_t *p_onu_uint,
                uint32_t *p_phy_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_onu_uint || !p_phy_uint)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint >= %d and onu_uint <= %d and online_flag = 1 order by phy_uint", *p_onu_uint, onu_uint_end);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_phy_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_PHY_UINT);
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get_by_desc(
                const char *p_desc,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_desc || !p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where desc like \'%s\'", p_desc);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_info_tbl_get(
                const uint32_t onu_uint,
                uint32_t *p_phy_uint,
                uint8_t *p_online_flag,
                uint8_t *p_deactive_flag,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_phy_uint || !p_online_flag || !p_deactive_flag || !p_auth_mode)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_info_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    ret = SQLITE_NOTFOUND;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if(sqlite3_column_text(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_PHY_UINT))
            *p_phy_uint = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_PHY_UINT);
        else
            *p_phy_uint = INVALID_PHY_UINT;
        *p_online_flag = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_ONLINE_FLAG);
        *p_deactive_flag = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_DEACTIVE);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_INFO_TBL_STMT_IDX_AUTH_MODE);
        ret = SQLITE_OK;
        break;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}

int
fk_onu_db_onu_auth_info_tbl_del(
                const uint32_t onu_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_auth_sn_pwd_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_auth_pwd_tbl where onu_uint = %d", onu_uint);

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_auth_loid_tbl where onu_uint = %d", onu_uint);

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_auth_info_tbl where onu_uint = %d", onu_uint);

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    NODE_DB_UNLOCK;

    return SQLITE_OK;
}


static int
fk_create_onu_auth_info_tbl(void)
{
    int ret;
    char * p_errmsg = NULL;

    ret = sqlite3_exec(p_fk_onu_db, "drop table if exists onu_auth_info_tbl", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "drop onu_auth_info_tbl failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "create table "
                                            "onu_auth_info_tbl("
                                                "onu_uint int primary key not null, "
                                                "phy_uint int unique, "
                                                "auth_mode int8, "
                                                "auth_time int , "
                                                "deactive int8 not null, "
                                                "online_flag int8, "
                                                "online_time int, "
                                                "offline_time int, "
                                                "dg_timg int, "
                                                "desc text unique, "
                                                "lineprofile_id int not null, "
                                                "srvprofile_id int not null, "
                                                "FOREIGN KEY(phy_uint) REFERENCES onu_phy_info_tbl(phy_uint))",
                                            NULL, NULL, &p_errmsg );
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "create onu_auth_info_tbl failed", ret);

    return SQLITE_OK;
}


#endif


#if DEFUNC("onu_auth_sn_pwd_tbl")


static int
fk_onu_db_onu_auth_check_expire(
                const FK_ONU_AUTH_TIME_MODE_E time_mode,
                const uint32_t expire_at,
                const uint8_t authed_flag)
{
    struct sysinfo sys_info;

    sysinfo(&sys_info);

    switch(time_mode)
    {
        case FK_ONU_AUTH_TIME_MODE_ALWAYS:
            return 0;

        case FK_ONU_AUTH_TIME_MODE_ONCE_NO_AGING:
            if(!authed_flag)
                return 0;
            break;

        case FK_ONU_AUTH_TIME_MODE_ONCE_AGING:
            if(!authed_flag && expire_at > sys_info.uptime)
                return 0;
            break;

        default:
            break;
    }

    return 1;
}


static int
fk_onu_db_onu_auth_sn_pwd_tbl_aging(
                const uint32_t onu_uint,
                uint8_t *p_aging_flag)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    uint32_t expire_at;
    uint8_t time_mode, authed_flag;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_sn_pwd_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE);
        expire_at = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT);
        authed_flag = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG);

        if(!fk_onu_db_onu_auth_check_expire(time_mode, expire_at, authed_flag))
            break;

        *p_aging_flag = 1;

        sqlite3_finalize(stmt);

        snprintf(cmd_str, sizeof(cmd_str),
            "delete from onu_auth_sn_pwd_tbl where onu_uint = %d", onu_uint);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        snprintf(cmd_str, sizeof(cmd_str),
            "delete from onu_auth_info_tbl where onu_uint = %d", onu_uint);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_OK;
}


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
                const uint32_t srvprofile_id)
{
    int ret;
    sqlite3_stmt *stmt;
    const char *sql = "insert into onu_auth_sn_pwd_tbl values(?, ?, ?, ?, ?, ?, 0)";
    char sn_str[MAX_SN_STR_SIZE];
    char pwd_str[MAX_PWD_STR_SIZE];
    struct sysinfo sys_info;
    uint32_t onu_uint = NODE_ID_2_NODE_UINT(tf_ni, onu_id);

    if(!VALID_NODE_UINT(onu_uint) || !p_serial_number)
        return SQLITE_PERM;

    sysinfo(&sys_info);

    switch(auth_mode)
    {
        case NODE_AUTH_MODE_SN:
            if(!p_serial_number || p_password)
                return SQLITE_PERM;
            break;
        case NODE_AUTH_MODE_SN_PWD:
            if(!p_serial_number || !p_password)
                return SQLITE_PERM;
            break;
        default:
           return SQLITE_PERM;
    }

    if(time_mode == FK_ONU_AUTH_TIME_MODE_ONCE_AGING && !sec_duration)
        return SQLITE_PERM;

    if(!fk_does_sn_offline(tf_ni, p_serial_number))
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, MAX_SN_STR_SIZE);

    ret = fk_onu_db_onu_auth_info_tbl_add(tf_ni, onu_id, auth_mode, lineprofile_id, srvprofile_id);
    if(ret)
        return ret;

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, sql, strlen(sql), &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 1, onu_uint);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 2, auth_mode);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_text(stmt, 3, sn_str, MAX_SN_STR_SIZE, NULL);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        if(p_password)
        {
            fk_pwd_to_str(p_password, pwd_str, MAX_PWD_STR_SIZE);
            ret = sqlite3_bind_text(stmt, 4, pwd_str, MAX_PWD_STR_SIZE, NULL);
            BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);
        }
        else
        {
            ret = sqlite3_bind_null(stmt, 4);
            BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);
        }

        ret = sqlite3_bind_int(stmt, 5, time_mode);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        if(time_mode != FK_ONU_AUTH_TIME_MODE_ONCE_AGING)
            ret = sqlite3_bind_null(stmt, 6);
        else
            ret = sqlite3_bind_int(stmt, 6, sys_info.uptime + sec_duration);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, sql, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;
    }
    while(0);

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}


int
fk_onu_db_onu_auth_sn_pwd_tbl_check(
                const char *p_serial_number,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret, rc = SQLITE_NOTFOUND;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    char sn_str[MAX_SN_STR_SIZE];
    char pwd_str[MAX_PWD_STR_SIZE];
    const unsigned char *p_pwd_str;
    uint32_t expire_at;
    uint8_t time_mode, authed_flag;

    if(!p_serial_number || !p_password || !p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_sn_pwd_tbl where sn like \'%s\'", sn_str);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE);
        expire_at = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT);
        authed_flag = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG);

        printf("%s:%d time_mode:%d expire_at:%d\r\n", __FUNCTION__, __LINE__,time_mode, expire_at);

        if(fk_onu_db_onu_auth_check_expire(time_mode, expire_at, authed_flag))
            break;

        printf("%s:%d time_mode:%d expire_at:%d\r\n", __FUNCTION__, __LINE__,time_mode, expire_at);

        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTH_MODE);
        p_pwd_str = sqlite3_column_text(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_PWD);
        if(*p_auth_mode == NODE_AUTH_MODE_SN_PWD)
        {
            if(fk_pwd_to_str(p_password, pwd_str, sizeof(pwd_str)) ||
                    strncmp((const char*)p_pwd_str, pwd_str, MAX_PWD_STR_SIZE))
                break;
        }

        sqlite3_finalize(stmt);

        //update authed_flag
        snprintf(cmd_str, sizeof(cmd_str),
            "update onu_auth_sn_pwd_tbl set authed_flag = 1 where onu_uint = %d", *p_onu_uint);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

#if 000
    if(aging_flag)
    {
        printf("%s:%d \r\n", __FUNCTION__, __LINE__);
        snprintf(cmd_str, sizeof(cmd_str),
            "delete from onu_auth_sn_pwd_tbl where sn like \'%s\'", sn_str);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, rc); //return rc, not ret ?

        snprintf(cmd_str, sizeof(cmd_str),
            "delete from onu_auth_info_tbl where onu_uint = %d", *p_onu_uint);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, rc); //return rc, not ret ?
    }
#endif

    return rc;
}

int
fk_onu_db_onu_auth_sn_pwd_tbl_set_authed(
                const uint32_t onu_uint)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "update onu_auth_sn_pwd_tbl set authed_flag = 1 where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    return SQLITE_OK;
}

int
fk_onu_db_onu_auth_sn_pwd_tbl_get_sn_pwd(
                const uint32_t onu_uint,
                uint8_t *p_auth_mode,
                char *p_serial_number,
                char *p_password)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    uint32_t expire_at;
    uint8_t time_mode, authed_flag;

    if(!p_auth_mode || !p_serial_number || !p_password)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_sn_pwd_tbl where onu_uint = %d", onu_uint);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE);
        expire_at = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT);
        authed_flag = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG);

        /* ignore aging_flag, so even if time mode is 'once', this code will never trigger aging */
        if(fk_onu_db_onu_auth_check_expire(time_mode, expire_at, authed_flag))
        {
            ret = SQLITE_NOTFOUND;
            break;
        }

        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTH_MODE);
        fk_str_to_sn(sqlite3_column_text(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_SN), p_serial_number);
        fk_str_to_pwd(sqlite3_column_text(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_PWD), p_password);
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_auth_sn_pwd_tbl_get_onu_uint_by_sn(
                const char *p_serial_number,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char sn_str[MAX_SN_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    uint32_t expire_at;
    uint8_t time_mode, authed_flag;

    if(!p_serial_number || !p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_sn_pwd_tbl where sn like \'%s\'", sn_str);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE);
        expire_at = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT);
        authed_flag = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG);

        /* ignore aging_flag, so even if time mode is 'once', this code will never trigger aging */
        if(fk_onu_db_onu_auth_check_expire(time_mode, expire_at, authed_flag))
        {
            ret = SQLITE_NOTFOUND;
            break;
        }

        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTH_MODE);
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_auth_sn_pwd_tbl_get_onu_uint_by_pwd_first(
                const uint32_t onu_uint_start,
                const char *p_password,
                uint32_t *p_onu_uint,
                uint8_t *p_auth_mode)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    char pwd_str[MAX_PWD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    uint32_t expire_at;
    uint8_t time_mode, authed_flag;

    if(!p_password || !p_onu_uint || !p_auth_mode)
        return SQLITE_PERM;

    fk_pwd_to_str(p_password, pwd_str, MAX_PWD_STR_SIZE);

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_auth_sn_pwd_tbl where pwd like \'%s\' and onu_uint >= %d", pwd_str, onu_uint_start);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_TIME_MODE);
        expire_at = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_EXPIRE_AT);
        authed_flag = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTHED_FLAG);

        /* ignore aging_flag, so even if time mode is 'once', this code will never trigger aging */
        if(fk_onu_db_onu_auth_check_expire(time_mode, expire_at, authed_flag))
        {
            ret = SQLITE_NOTFOUND;
            break;
        }

        *p_onu_uint = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_ONU_UINT);
        *p_auth_mode = sqlite3_column_int(stmt, ONU_AUTH_SN_PWD_TBL_STMT_IDX_AUTH_MODE);
        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}

static int
fk_create_onu_auth_sn_pwd_tbl(void)
{
    int ret;
    char * p_errmsg = NULL;

    ret = sqlite3_exec(p_fk_onu_db, "drop table if exists onu_auth_sn_pwd_tbl", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "drop onu_auth_sn_pwd_tbl failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "create table "
                                            "onu_auth_sn_pwd_tbl("
                                                "onu_uint int primary key not null, "
                                                "auth_mode int not null, "
                                                "sn text unique not null COLLATE NOCASE, "
                                                "pwd text, "
                                                "time_mode int8, "
                                                "expire_at int, "
                                                "authed_flag int8, "
                                                //"constraint auth_key primary key(sn, pwd), "
                                                "FOREIGN KEY(onu_uint) REFERENCES onu_auth_info_tbl(onu_uint))",
                                            NULL, NULL, &p_errmsg );
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "create onu_auth_sn_pwd_tbl failed", ret);

    return SQLITE_OK;
}

#endif

#if DEFUNC("onu_filter_tbl")

int
fk_onu_db_onu_fliter_tbl_set_switch(
                uint8_t sw_val)
{
    g_onu_filter_info.sw_val = sw_val ? 1 : 0;
    return SQLITE_OK;
}

int
fk_onu_db_onu_fliter_tbl_get_switch(
                uint8_t *p_sw)
{
    if (!p_sw)
        return SQLITE_PERM;

    *p_sw = g_onu_filter_info.sw_val;
    return SQLITE_OK;
}

int
fk_onu_db_onu_fliter_tbl_set_mode(
                uint8_t mode)
{
    g_onu_filter_info.mode = mode;
    return SQLITE_OK;
}

int
fk_onu_db_onu_fliter_tbl_get_mode(
                uint8_t *p_mode)
{
    if (!p_mode)
        return SQLITE_PERM;

    *p_mode = g_onu_filter_info.mode;
    return SQLITE_OK;
}

int
fk_onu_db_onu_fliter_tbl_get_records_count(
                uint32_t *p_count)
{
    if (!p_count)
        return SQLITE_PERM;

    *p_count = g_onu_filter_info.count;
    return SQLITE_OK;
}


static int
fk_onu_db_onu_filter_tbl_conflict_check(
                const uint8_t mask_len,
                const char *p_sn_str)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_sn_str)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_filter_tbl where sn like \'%c%c%c%c%%\'", p_sn_str[0], p_sn_str[1], p_sn_str[2], p_sn_str[3]);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        uint8_t record_mask_len = sqlite3_column_int(stmt, 1);
        uint8_t compare_mask_len;
        const char *p_record_sn_str = (const char *)sqlite3_column_text(stmt, 0);

        compare_mask_len = record_mask_len < mask_len ? record_mask_len : mask_len;
        if(strncmp((const char*)p_record_sn_str+4, p_sn_str+4, compare_mask_len-4))
            continue;

        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_fliter_tbl_add(
                const uint8_t mask_len,
                const char *p_sn_str)
{
    int ret;
    sqlite3_stmt *stmt;
    const char* sql = "insert into onu_filter_tbl values(?, ?, 0, NULL)";
    char sn_str[MAX_SN_STR_SIZE];

    if (mask_len > 12 || mask_len < 4)
        return SQLITE_PERM;

    if(!p_sn_str)
        return SQLITE_PERM;

    if(g_onu_filter_info.count >= MAX_ONU_FILTER_NUM)
        return SQLITE_PERM;

    if(!fk_onu_db_onu_filter_tbl_conflict_check(mask_len, p_sn_str))
        return SQLITE_PERM;

    snprintf(sn_str, mask_len+1, "%s", p_sn_str);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, "begin;", 0, 0, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

    do
    {
        ret = sqlite3_prepare_v2(p_fk_onu_db, sql, strlen(sql), &stmt, 0);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_reset(stmt);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_text(stmt, 1, sn_str, sizeof(sn_str), NULL);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_bind_int(stmt, 2, mask_len);
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, sql, ret);

        ret = sqlite3_step( stmt );
        BREAK_ON_SQLITE(p_fk_onu_db, ret, SQLITE_DONE, sql, ret);

        sqlite3_exec(p_fk_onu_db,"commit;", 0, 0, 0);
        ret = SQLITE_OK;

        g_onu_filter_info.count++;
    }
    while(0);

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return ret;
}

int
fk_onu_db_onu_filter_tbl_del(
                const uint16_t offset) //start from 0
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_filter_tbl where idx in(select idx from onu_filter_tbl limit 1 offset %d);", offset);

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    g_onu_filter_info.count--;

    return SQLITE_OK;
}

int
fk_onu_db_onu_filter_tbl_del_all(void)
{
     int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];

    snprintf(cmd_str, sizeof(cmd_str),
        "delete from onu_filter_tbl");

    NODE_DB_LOCK;

    ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);

    NODE_DB_UNLOCK;

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    g_onu_filter_info.count = 0;

    return SQLITE_OK;
}

//search the sn in db
int
fk_onu_db_onu_filter_tbl_sn_hit_check(
                const char *p_serial_number)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;
    int mask_len;
    char sn_str[MAX_SN_STR_SIZE];
    int idx;

    if(!p_serial_number)
        return SQLITE_PERM;

    fk_sn_to_str(p_serial_number, sn_str, sizeof(sn_str));

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_filter_tbl where sn like \'%c%c%c%c%%\'", sn_str[0], sn_str[1], sn_str[2], sn_str[3]);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        mask_len = sqlite3_column_int(stmt, 1);
        const char *p_record_sn_str = (const char *)sqlite3_column_text(stmt, 0);

        if(strncmp((const char*)p_record_sn_str+4, sn_str+4, mask_len-4))
            continue;

        idx = sqlite3_column_int(stmt, 3);
        sqlite3_finalize(stmt);

        //sn hit ,increase the hit_count
        snprintf(cmd_str, sizeof(cmd_str),
                "update onu_filter_tbl set count=count+1 where idx = %d", idx);

        ret = sqlite3_exec(p_fk_onu_db, cmd_str, 0, 0, 0);
        EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


int
fk_onu_db_onu_filter_tbl_get_first(
                const int start_idx,
                int *p_idx,
                char *p_sn_str,
                uint8_t *p_sn_mask_len,
                uint32_t*p_hit_count)
{
    int ret;
    char cmd_str[MAX_SQLITE_CMD_STR_SIZE];
    sqlite3_stmt *stmt = NULL;

    if(!p_sn_str || !p_sn_mask_len || !p_hit_count)
        return SQLITE_PERM;

    snprintf(cmd_str, sizeof(cmd_str),
        "select * from onu_filter_tbl where idx >= %d order by idx", start_idx);

    NODE_DB_LOCK;

    ret = sqlite3_prepare_v2(p_fk_onu_db, cmd_str, -1, &stmt, 0);
    EXIT_ON_SQLITE_UNLOCK(p_fk_onu_db, ret, SQLITE_OK, cmd_str, ret);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        strncpy(p_sn_str, (const char*)sqlite3_column_text(stmt, 0), 12);  //TODO 12->macro
        *p_sn_mask_len = sqlite3_column_int(stmt, 1);
        *p_hit_count = sqlite3_column_int(stmt, 2);
        *p_idx = sqlite3_column_int(stmt, 3);

        sqlite3_finalize(stmt);
        NODE_DB_UNLOCK;

        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    NODE_DB_UNLOCK;

    return SQLITE_NOTFOUND;
}


static int
fk_create_onu_filter_tbl(void)
{
    int ret;
    char * p_errmsg = NULL;

    ret = sqlite3_exec(p_fk_onu_db, "drop table if exists onu_filter_tbl", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "drop onu_filter_tbl failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "create table "
                                            "onu_filter_tbl("
                                                "sn text unique not null COLLATE NOCASE, "
                                                "mask_len int,"
                                                "count int,"
                                                "idx INTEGER PRIMARY KEY)",
                                                NULL, NULL, &p_errmsg );

    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "create onu_filter_tbl failed", ret);

    return SQLITE_OK;
}


#endif

#if DEFUNC("外部接口函数")


int
fk_db_init(void)
{
    int ret;

    if (sem_init(&fk_onu_db_sem, 0, 1) != 0 )
    {
        printf("Init sem for db failed\r\n");
        return -1;
    }

    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    //ret = sqlite3_open(":tfdrv_db:", &p_fk_onu_db);
    ret = sqlite3_open_v2("tfdrv_db", &p_fk_onu_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "Open db failed", ret);

    //sqlite3_config(SQLITE_CONFIG_MULTITHREAD);

    //ret = sqlite3_exec(p_fk_onu_db, "PRAGMA synchronous = OFF; ", 0, 0, 0);
    //EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "Init db failed", ret);

    ret = sqlite3_exec(p_fk_onu_db, "PRAGMA foreign_keys = ON; ", 0, 0, 0);
    EXIT_ON_SQLITE(p_fk_onu_db, ret, SQLITE_OK, "Init db failed", ret);

    autofind_aging_duration = DEFAULT_AUTOFIND_AGING_DURATION;

    fk_create_onu_black_tbl();
    fk_create_onu_phy_info_tbl();

    fk_create_onu_auth_sn_pwd_tbl();
    fk_create_onu_auth_info_tbl();

    fk_create_onu_filter_tbl();

    return 0;
}

int
fk_onu_db_uninit(void)
{
    return 0;
}


#endif

