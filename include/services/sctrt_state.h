#ifndef _SCTRT_STATE_H
#define _SCTRT_STATE_H

#include <linux/types.h>

int sctrt_state_init(int max_syscalls);
void sctrt_state_cleanup(void);

void sctrt_monitor_enable(void);
void sctrt_monitor_disable(void);
bool sctrt_monitor_is_active(void);

// int sctrt_max_set(unsigned int max);
// int sctrt_max_get();

// int sctrt_euid_register(uid_t euid);
// int sctrt_euid_deregister(uid_t euid);

// int sctrt_syscall_register(int syscall_nr);
// int sctrt_syscall_deregister(int syscall_nr);

#endif