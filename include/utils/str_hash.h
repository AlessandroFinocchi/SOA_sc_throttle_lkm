#ifndef STR_HASH_H
#define STR_HASH_H

#include <linux/types.h>

#define STR_HASH_BITS 8

struct string_hash {
    spinlock_t lock;
    struct hlist_head table[1 << STR_HASH_BITS];
};

int str_hash_init(struct string_hash *hash);
int str_hash_add(struct string_hash *hash, char *str);
bool str_hash_lookup(struct string_hash *hash, char *str);
void str_hash_del(struct string_hash *hash, char *str);
void str_hash_cleanup(struct string_hash *hash);
void str_hash_print(struct string_hash *hash);

#endif