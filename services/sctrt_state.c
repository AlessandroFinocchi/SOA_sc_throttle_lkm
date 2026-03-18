#include <linux/slab.h>
#include <linux/errno.h>

#include "sctrt_state.h"
#include "sc_bitmap.h"
#include "euid_hash.h"
#include "str_hash.h"

struct sctrt_state {
    bool is_active;
    uint MAX;
    struct sc_bitmap *syscalls;
    struct string_hash *programs;
    struct euid_hash *users;
};

struct sctrt_state *state;


int sctrt_state_init(int max_syscalls) {
    int status;

    if (!(state = kmalloc(sizeof(struct sctrt_state), GFP_KERNEL))) {
        status = -ENOMEM;
        goto end;
    }

    if ((status = str_hash_init(&state->programs))) {
        goto delete_state;
    }

    if ((status = euid_hash_init(&state->users))) {
        goto delete_state_programs;
    }

    if ((status = sc_bitmap_init(&state->syscalls, max_syscalls))) {
        goto delete_state_programs_users;
    }

    state->is_active = false;
    state->MAX = 0;
    return 0;

delete_state_programs_users:
    euid_hash_cleanup(state->users);
delete_state_programs:
    str_hash_cleanup(state->programs);
delete_state:
    kfree(state);
end:
    return status;
}

void sctrt_state_cleanup() {
    /* Assicurarsi di aver terminato tutte le lookup */
    sc_bitmap_cleanup(state->syscalls);
    euid_hash_cleanup(state->users);
    str_hash_cleanup(state->programs);
    kfree(state);
}


void sctrt_monitor_enable() {
    state->is_active = true;
}

void sctrt_monitor_disable() {
    state->is_active = false;
}

bool sctrt_monitor_is_active() {
    return state->is_active;
}

void sctrt_max_set(unsigned int max) {
    state->MAX = max;
}

int sctrt_syscall_register(int syscall_nr) {
    return sc_bitmap_register(state->syscalls, syscall_nr);
}

int sctrt_syscall_deregister(int syscall_nr) {
    return sc_bitmap_unregister(state->syscalls, syscall_nr);
}

int sctrt_euid_register(uid_t euid) {
    kuid_t euid_k = KUIDT_INIT(euid);
    return euid_hash_add(state->users, euid_k);
}

int sctrt_euid_deregister(uid_t euid) {
    kuid_t euid_k = KUIDT_INIT(euid);
    return euid_hash_del(state->users, euid_k);
}

int sctrt_prog_register(char *prog_name) {
    return str_hash_add(state->programs, prog_name);
}

int sctrt_prog_deregister(char *prog_name) {
    return str_hash_del(state->programs, prog_name);
}


void sctrt_print_state() {
    printk("%s: Stato del monitor: %s\n", MODNAME, 
        sctrt_monitor_is_active() ? "attivo" : "spento");
}

void sctrt_print_max() {
    printk("%s: Configurazion di MAX: %u\n", MODNAME, state->MAX);
}

void sctrt_print_syscalls() {
    sc_bitmap_print(state->syscalls);
}

void sctrt_print_users() {
    euid_hash_print(state->users);
}

void sctrt_print_programs() {
    str_hash_print(state->programs);
}