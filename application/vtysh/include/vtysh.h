/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef VTYSH_H
#define VTYSH_H

#include "global.h"

#define VTYSH_TEST    (1<<0)
#define VTYSH_ALARM   (1<<1)
#define VTYSH_QOSACL  (1<<2)
#define VTYSH_GTF    (1<<3)
#define VTYSH_LAYER2  (1<<4)
#define VTYSH_RSTP    (1<<5)
#define VTYSH_IGMPSN  (1<<6)
#define VTYSH_DHCPSN  (1<<7)
#define VTYSH_HAL     (1<<8)
#define VTYSH_SYSCTRL (1<<9)
#define VTYSH_LACP    (1<<10)
#define VTYSH_DOT1X   (1<<11)


#define VTYSH_ALL	  (VTYSH_TEST|VTYSH_ALARM|VTYSH_QOSACL|VTYSH_GTF| \
                       VTYSH_LAYER2|VTYSH_RSTP|VTYSH_IGMPSN|VTYSH_DHCPSN| \
                       VTYSH_HAL|VTYSH_SYSCTRL|VTYSH_LACP|VTYSH_DOT1X)

#define VTYSH_PORT 23

#define VTYSH_ENABLE

/* vtysh local configuration file. */
#define VTYSH_DEFAULT_CONFIG "vtysh.conf"

void vtysh_init_vty (void);
void vtysh_init_cmd (void);
extern int vtysh_connect_all (const char *optional_daemon_name);
void vtysh_readline_init (void);
void vtysh_user_init (void);

int vtysh_execute (const char *);
int vtysh_execute_no_pager (const char *);
int vtysh_cmd_execute (struct vty *vty, const char *cmd);

char *vtysh_prompt (void);

void vtysh_config_write (void);

int vtysh_config_from_file (struct vty *, FILE *);

int vtysh_read_config (char *);

void vtysh_config_parse (char *);

void vtysh_config_dump (FILE *);

void vtysh_config_init (void);

void vtysh_pager_init (void);

void vtysh_set_timeout_flag(bool s);

void
vtysh_timeout(int u);

void
vty_user_init ();
void vty_user_load(void);
void vty_user_save(void);

void vtysh_init();

void create_thread_no_arg(void *func);

int vtysh_node_get(const char *line);

int vtysh_config_header(struct vty *vty, FILE *fp);

void vtysh_config_start();

/* Child process execution flag. */
extern int execute_flag;

extern struct vty *vtysh;

typedef int (*vtysh_cmd_callback)(struct vty * vty, char *cmd);
extern vtysh_cmd_callback g_vtysh_exec_command;
#endif /* VTYSH_H */
