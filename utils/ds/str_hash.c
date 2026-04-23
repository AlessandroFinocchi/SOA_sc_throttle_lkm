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

int str_hash_init(struct string_hash **hash) {
    if (!hash)
        return -EINVAL;

    if(!(*hash = kzalloc(sizeof(struct string_hash), GFP_KERNEL))) {
        return -ENOMEM;
    }

    spin_lock_init(&(*hash)->lock);
    hash_init((*hash)->table);
    return 0;
}

int str_hash_add(struct string_hash *hash, char *new_str) {
    struct string_node *new_node;
    u32 h;
    size_t len;

    if (!hash || !new_str)
        return -EINVAL;
    
    if (str_hash_lookup(hash, new_str))
        return -EINVAL;

    len = strlen(new_str) + 1;
    // Allocazione in process ctx., usa GFP_ATOMIC se in interrupt ctx.
    new_node = kmalloc(sizeof(*new_node) + len, GFP_KERNEL); 
    if (!new_node)
        return -ENOMEM;

    strscpy(new_node->str, new_str, len);
    h = hash_string(new_str);

    // I lettori (in interrupt ctx) non acquisiscono mai lo spinlock,
    // quindi non serve spin_lock_irqsave()
    spin_lock(&hash->lock);
    hash_add_rcu(hash->table, &new_node->node, h);
    spin_unlock(&hash->lock);

    return 0;
}

/* 
 * Eseguita in interrupt ctx: rcu_read_lock/unlock implementa un meccanismo
 * wait-free per i lettori, e la coerenza rispetto ad add/delete è garantita
 * dal loro utilizzo di hash_add_rcu e hlist_del_rcu.
 */
bool str_hash_lookup(struct string_hash *hash, char *target) {
    struct string_node *curr;
    u32 h;
    bool found = false;

    if (!hash || !target)
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

int str_hash_del(struct string_hash *hash, char *target) {
    struct string_node *curr;
    u32 h;

    if (!hash || !target)
        return -EINVAL;

    h = hash_string(target);

    // I lettori (in interrupt ctx) non acquisiscono mai lo spinlock,
    // quindi non serve spin_lock_irqsave()
    spin_lock(&hash->lock);
    hash_for_each_possible(hash->table, curr, node, h) {
        if (strcmp(curr->str, target) == 0) {
            hlist_del_rcu(&curr->node);
            spin_unlock(&hash->lock);
            kfree_rcu(curr, rcu);
            return 0;
        }
    }
    spin_unlock(&hash->lock);
    return -EINVAL;
}

void str_hash_cleanup(struct string_hash *hash) {
    struct string_node *curr;
    struct hlist_node *tmp;
    int bkt;

    if (!hash)
        return;

    spin_lock(&hash->lock);
    hash_for_each_safe(hash->table, bkt, tmp, curr, node) {
        hlist_del_rcu(&curr->node);
        kfree_rcu(curr, rcu);
    }
    spin_unlock(&hash->lock);

    rcu_barrier();

    kfree(hash);
}

void str_hash_print(struct string_hash *hash) {
    struct string_node *curr;
    int bkt;

    if (!hash)
        return;

    rcu_read_lock();
    pr_info("STR_HASH: Registered programs:");
    hash_for_each_rcu(hash->table, bkt, curr, node) {
        pr_info("\t\t - %s", curr->str);
    }
    pr_cont("\n");
    rcu_read_unlock();
}