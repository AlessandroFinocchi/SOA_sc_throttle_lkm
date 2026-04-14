#include <linux/cred.h> // For current macro
#include <asm/syscall.h>      /* Fornisce syscall_get_nr() */

#include "sctrt_core.h"
#include "sctrt_state.h"
#include "sc_bitmap.h"
#include "euid_hash.h"
#include "str_hash.h"

bool sctrt_check_throttling_compatibility(struct pt_regs *regs) {
    long syscall_nr;
    char *progname;
    kuid_t current_uid;

    syscall_nr = syscall_get_nr(current, (struct pt_regs *)regs->di);
    progname = current->comm;
    current_uid = current_euid();

    /* 1. syscall check */
    if(!sc_bitmap_lookup(state->syscalls, syscall_nr)){
        return false;
    }

    /* * 2. process name check*/
    if(!str_hash_lookup(state->programs, progname)) {
        return false;
    }

    /* * 3. euid check*/
    if (!euid_hash_lookup(state->users, current_uid)) {
        return false;
    }

    return true;
}