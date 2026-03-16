#include <linux/module.h>
#include <linux/hashtable.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/jhash.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "str_hash.h"

struct string_node {
    struct hlist_node node;
    struct rcu_head rcu;
    char str[];
};

static inline u32 hash_string(char *str) {
    return jhash(str, strlen(str), 0);
}

int str_hash_init(struct string_hash *hash) {
    if (!hash)
        return -EINVAL;

    spin_lock_init(&hash->lock);
    hash_init(hash->table);
    return 0;
}

int str_hash_add(struct string_hash *hash, char *new_str) {
    struct string_node *new_node;
    u32 h;
    size_t len;

    if (!hash || !new_str)
        return -EINVAL;

    len = strlen(new_str) + 1;
    // Allocazione in process ctx., usa GFP_ATOMIC se in interrupt ctx.
    new_node = kmalloc(sizeof(*new_node) + len, GFP_KERNEL); 
    if (!new_node)
        return -ENOMEM;

    strscpy(new_node->str, new_str, len);
    h = hash_string(new_str);

    spin_lock(&hash->lock);
    hash_add_rcu(hash->table, &new_node->node, h);
    spin_unlock(&hash->lock);

    return 0;
}

bool str_hash_lookup(struct string_hash *hash, char *target) {
    struct string_node *curr;
    u32 h;
    bool found = false;

    if (!hash ||!target)
        return false;

    h = hash_string(target);

    rcu_read_lock();
    hash_for_each_possible_rcu(hash->table, curr, node, h) {
        if (strcmp(curr->str, target) == 0) {
            found = true;
            break;
        }
    }
    rcu_read_unlock();

    return found;
}

void str_hash_del(struct string_hash *hash, char *target) {
    struct string_node *curr;
    u32 h;

    if (!hash || !target)
        return;

    h = hash_string(target);

    spin_lock(&hash->lock);
    hash_for_each_possible(hash->table, curr, node, h) {
        if (strcmp(curr->str, target) == 0) {
            hlist_del_rcu(&curr->node);
            spin_unlock(&hash->lock);
            kfree_rcu(curr, rcu);
            return;
        }
    }
    spin_unlock(&hash->lock);
}

void str_hash_cleanup(struct string_hash *hash) {
    struct string_node *curr;
    struct hlist_node *tmp;
    int bkt;

    if (!hash)
        return;

    spin_lock(&hash->lock);
    hash_for_each_safe(hash->table, bkt, tmp, curr, node) {
        hash_del_rcu(&curr->node);
        kfree_rcu(curr, rcu);
    }
    spin_unlock(&hash->lock);

    rcu_barrier();
}

void str_hash_print(struct string_hash *hash) {
    struct string_node *curr;
    int bkt;

    if (!hash)
        return;

    rcu_read_lock();
    hash_for_each(hash->table, bkt, curr, node) {
        pr_info("STR_HASH: str=%s", curr->str);
    }
    rcu_read_unlock();
}