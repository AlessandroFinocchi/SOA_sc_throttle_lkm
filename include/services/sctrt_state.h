#ifndef _SCTRT_STATE_H
#define _SCTRT_STATE_H

#include <linux/types.h>

#define MODNAME "SCTRT"

struct sctrt_state {
    /* Monitor state */
    bool is_active;
    uint MAX;
    struct sc_bitmap *syscalls;
    struct string_hash *programs;
    struct euid_hash *users;
};

extern struct sctrt_state *state;

int sctrt_state_init(void);
void sctrt_state_cleanup(void);

void sctrt_monitor_enable(void);
void sctrt_monitor_disable(void);
bool sctrt_monitor_is_active(void);
void sctrt_print_state(void);

void sctrt_max_set(unsigned int max);
void sctrt_print_max(void);

int sctrt_syscall_register(int syscall_nr);
int sctrt_syscall_deregister(int syscall_nr);
void sctrt_print_syscalls(void);

int sctrt_euid_register(uid_t euid);
int sctrt_euid_deregister(uid_t euid);
void sctrt_print_users(void);

int sctrt_prog_register(char *prog_name);
int sctrt_prog_deregister(char *prog_name);
void sctrt_print_programs(void);

#endif