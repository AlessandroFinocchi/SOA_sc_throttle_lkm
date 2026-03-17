#include <linux/slab.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/errno.h>

#include "sc_bitmap.h"

int sc_bitmap_create(struct sc_bitmap *syscalls, unsigned int max_syscalls) {
    if (!(syscalls = kmalloc(sizeof(struct sc_bitmap), GFP_KERNEL))) {
        return -ENOMEM;
    }

    if (!(syscalls->bitmap = bitmap_zalloc(max_syscalls, GFP_KERNEL))) {
        kfree(syscalls);
        return -ENOMEM;
    }

    syscalls->max_entries = max_syscalls;
    return 0;
}

int sc_bitmap_register(struct sc_bitmap *syscalls, unsigned int syscall_nr) {
    if (!syscalls || syscall_nr >= syscalls->max_entries) {
        return -EINVAL;
    }
    set_bit(syscall_nr, syscalls->bitmap);
    return 0;
}

int sc_bitmap_unregister(struct sc_bitmap *syscalls, unsigned int syscall_nr) {
    if (!syscalls || syscall_nr >= syscalls->max_entries) {
        return -EINVAL;
    }
    clear_bit(syscall_nr, syscalls->bitmap);
    return 0;
}

int sc_bitmap_lookup(struct sc_bitmap *syscalls, unsigned int syscall_nr) {
    if (!syscalls || syscall_nr >= syscalls->max_entries) {
        return 0;
    }
    return test_bit(syscall_nr, syscalls->bitmap) != 0;
}

void sc_bitmap_cleanup(struct sc_bitmap *syscalls) {
    if (!syscalls) {
        return;
    }

    if (syscalls->bitmap) {
        bitmap_free(syscalls->bitmap); 
        syscalls->bitmap = NULL; 
    }

    kfree(syscalls);
}