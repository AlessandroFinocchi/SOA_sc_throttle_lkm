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

    // if(!(state->syscalls = kmalloc(sizeof(struct sc_bitmap), GFP_KERNEL))) {
    //     status = -ENOMEM;
    //     kfree(state);
    //     goto end;
    // }

    if(!(state->programs = kzalloc(sizeof(struct string_hash), GFP_KERNEL))) {
        status = -ENOMEM;
        // kfree(state->syscalls);
        kfree(state);
        goto end;
    }

    if(!(state->users = kzalloc(sizeof(struct euid_hash), GFP_KERNEL))) {
        status = -ENOMEM;
        kfree(state->programs);
        // kfree(state->syscalls);
        kfree(state);
        goto end;
    }

    if ((status = str_hash_init(state->programs))) {
        kfree(state->users);
        kfree(state->programs);
        // kfree(state->syscalls);
        kfree(state);
        goto end;
    }

    if ((status = euid_hash_init(state->users))) {
        str_hash_cleanup(state->programs);
        kfree(state->users);
        kfree(state->programs);
        // kfree(state->syscalls);
        kfree(state);
        goto end;
    }

    if ((status = sc_bitmap_create(&state->syscalls, max_syscalls))) {
        euid_hash_cleanup(state->users);
        str_hash_cleanup(state->programs);
        kfree(state->users);
        kfree(state->programs);
        // kfree(state->syscalls);
        kfree(state);
        goto end;
    }

    state->is_active = false;
    state->MAX = 0;

    return 0;

end:
    return status;
}

void sctrt_state_cleanup() {
    printk("AAAAAAAAA1");
    sc_bitmap_cleanup(state->syscalls);
    printk("AAAAAAAAA2");
    euid_hash_cleanup(state->users);
    printk("AAAAAAAAA3");
    str_hash_cleanup(state->programs);
    printk("AAAAAAAAA4");
    kfree(state->users);
    printk("AAAAAAAAA5");
    kfree(state->programs);
    printk("AAAAAAAAA6");
    // kfree(state->syscalls);
    // printk("AAAAAAAAA7");
    kfree(state);
    printk("AAAAAAAAA8");
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