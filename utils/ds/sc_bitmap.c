#include <linux/slab.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/errno.h>

#include "sc_bitmap.h"

/* Sincronizzazione garantita dalle operazioni RMW */

int sc_bitmap_init(struct sc_bitmap **syscalls_ptr, unsigned int max_syscalls) {
    if (!syscalls_ptr)
        return -EINVAL;

    if (!(*syscalls_ptr = kmalloc(sizeof(struct sc_bitmap), GFP_KERNEL))) {
        return -ENOMEM;
    }

    if (!((*syscalls_ptr)->bitmap = bitmap_zalloc(max_syscalls, GFP_KERNEL))) {
        kfree(*syscalls_ptr);
        return -ENOMEM;
    }

    (*syscalls_ptr)->max_entries = max_syscalls;
    return 0;
}

int sc_bitmap_register(struct sc_bitmap *syscalls, unsigned int syscall_nr) {
    printk("BITM_DEMO: %d.\n", syscalls->max_entries);

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

void sc_bitmap_print(struct sc_bitmap *syscalls) {
    if (!syscalls) {
        return;
    }

    pr_info("SC_BITMAP: syscalls registrate:");
    for (int i = 0; i < syscalls->max_entries; i++) {
        if (test_bit(i, syscalls->bitmap))
            pr_cont(" %u", i);
    }
    pr_cont("\n");
}