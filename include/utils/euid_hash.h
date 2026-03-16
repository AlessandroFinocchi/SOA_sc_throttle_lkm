#ifndef EUID_HASH_H
#define EUID_HASH_H

#include <linux/types.h>
#include <linux/uidgid.h>

#define EUID_HASH_BITS 8

struct euid_hash {
    spinlock_t lock;
    struct hlist_head table[1 << EUID_HASH_BITS];
};

int euid_hash_init(struct euid_hash *hash);
int euid_hash_add(struct euid_hash *hash, kuid_t euid);
bool euid_hash_lookup(struct euid_hash *hash, kuid_t euid);
void euid_hash_del(struct euid_hash *hash, kuid_t euid);
void euid_hash_cleanup(struct euid_hash *hash);
void euid_hash_print(struct euid_hash *hash);

#endif
