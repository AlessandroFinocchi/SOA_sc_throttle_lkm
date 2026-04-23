#include <linux/cred.h>       // Fornisce la macro current
#include <asm/syscall.h>      /* Fornisce syscall_get_nr() */

#include "sctrt_core.h"
#include "sctrt_state.h"

bool sctrt_check_throttling_compatibility(struct pt_regs *regs) {
    long syscall_nr;
    char *prog_name;
    kuid_t current_euid;

    syscall_nr = syscall_get_nr(current, (struct pt_regs *)regs->di);
    prog_name = current->comm;
    current_euid = current_euid();

    /* 1. syscall check */
    if(!sctrt_is_syscall_registered(syscall_nr)){
        return false;
    }

    /* * 2. process name check*/
    if(!sctrt_is_prog_registered(prog_name)) {
        return false;
    }

    /* * 3. euid check*/
    if (!sctrt_is_euid_registered(current_euid)) {
        return false;
    }

    return true;
}