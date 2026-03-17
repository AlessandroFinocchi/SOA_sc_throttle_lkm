#ifndef SC_BITMAP_H
#define SC_BITMAP_H

#include <linux/types.h>

struct sc_bitmap {
    unsigned long *bitmap;
    unsigned int max_entries;
};

int sc_bitmap_create(struct sc_bitmap **syscalls, unsigned int max_syscalls);
int sc_bitmap_register(struct sc_bitmap *syscalls, unsigned int syscall_nr);
int sc_bitmap_unregister(struct sc_bitmap *syscalls, unsigned int syscall_nr);
int sc_bitmap_lookup(struct sc_bitmap *syscalls, unsigned int syscall_nr);
void sc_bitmap_cleanup(struct sc_bitmap *syscalls);

#endif