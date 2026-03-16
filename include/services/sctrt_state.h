#ifndef _SCTRT_STATE_H
#define _SCTRT_STATE_H

#include <linux/types.h>

int sctrt_state_init(void);

int sctrt_enable_monitor();
int sctrt_disable_monitor();

int sctrt_set_max(unsigned int max);
int sctrt_get_max();

int sctrt_register_euid(uid_t euid);
int sctrt_deregister_euid(uid_t euid);

int sctrt_register_syscall(int syscall_nr);
int sctrt_deregister_syscall(int syscall_nr);

#endif