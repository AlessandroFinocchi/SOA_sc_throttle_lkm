#include <linux/slab.h>
#include <linux/errno.h>

#include "sctrt_state.h"
#include "sc_bitmap.h"
#include "euid_hash.h"
#include "str_hash.h"

struct sctrt_state {
    bool is_active;
    uint MAX;
    // struct sc_bitmap *syscalls;
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

    if (!(status = str_hash_init(state->programs))) {
        goto delete_state;
    }

    if (!(status = euid_hash_init(state->users))) {
        goto delete_state_programs;
    }

    // if (!(status = sc_bitmap_create(state->syscalls, max_syscalls))) {
    //     goto delete_state_programs_users;
    // }

    state->is_active = false;
    state->MAX = 0;

    return 0;

// delete_state_programs_users:
//     euid_hash_cleanup(state->users);

delete_state_programs:
    str_hash_cleanup(state->programs);

delete_state:
    kfree(state);

end:
    return -ENOMEM;
}

void sctrt_state_cleanup() {
    euid_hash_cleanup(state->users);
    str_hash_cleanup(state->programs);
    // sc_bitmap_cleanup(state->syscalls);
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