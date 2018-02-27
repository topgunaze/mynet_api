
#ifndef _VTY_USER_H_
#define _VTY_USER_H_

struct vty_user *vty_user_lookup (const char *name);
bool vty_user_check_existed(const char *name);
struct vty_user *vty_user_check_passwd (const char *name, const char *passwd);
vty_user_info_login *vty_user_add_to_host_hist(struct vty_user *user);
bool vty_user_check_print_log_flag(const char *name, char bit);
void vty_user_init ();
void vty_user_load(void);
void vty_user_save(void);

#endif

