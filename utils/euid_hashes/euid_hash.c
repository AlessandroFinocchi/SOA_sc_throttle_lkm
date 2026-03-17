#include <linux/module.h>
#include <linux/hashtable.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "euid_hash.h"

struct euid_node {
    struct hlist_node node;
    struct rcu_head rcu;
    kuid_t euid;
};

static inline u32 hash_euid(kuid_t euid) {
    return __kuid_val(euid);
}

int euid_hash_init(struct euid_hash *hash) {
    if (!hash)
        return -EINVAL;

    spin_lock_init(&hash->lock);
    hash_init(hash->table);
    return 0;
}

int euid_hash_add(struct euid_hash *hash, kuid_t euid) {
    struct euid_node *new_node;
    u32 h;

    if (!hash)
        return -EINVAL;

    new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
    if (!new_node)
        return -ENOMEM;

    new_node->euid = euid;
    h = hash_euid(euid);

    spin_lock(&hash->lock);
    hash_add_rcu(hash->table, &new_node->node, h);
    spin_unlock(&hash->lock);

    return 0;
}

bool euid_hash_lookup(struct euid_hash *hash, kuid_t euid) {
    struct euid_node *curr;
    u32 h;
    bool found = false;

    if (!hash)
        return false;

    h = hash_euid(euid);

    rcu_read_lock();
    hash_for_each_possible_rcu(hash->table, curr, node, h) {
        if (uid_eq(curr->euid, euid)) {
            found = true;
            break;
        }
    }
    rcu_read_unlock();

    return found;
}

void euid_hash_del(struct euid_hash *hash, kuid_t euid) {
    struct euid_node *curr;
    u32 h;

    if (!hash)
        return;

    h = hash_euid(euid);

    spin_lock(&hash->lock);
    hash_for_each_possible(hash->table, curr, node, h) {
        if (uid_eq(curr->euid, euid)) {
            hlist_del_rcu(&curr->node); 
            spin_unlock(&hash->lock);
            kfree_rcu(curr, rcu);
            return;
        }
    }
    spin_unlock(&hash->lock);
}

void euid_hash_cleanup(struct euid_hash *hash) {
    struct euid_node *curr;
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
}

void euid_hash_print(struct euid_hash *hash) {
    struct euid_node *curr;
    int bkt;

    if (!hash)
        return;

    rcu_read_lock();
    hash_for_each_rcu(hash->table, bkt, curr, node) {
        pr_info("EUID_HASH: euid=%u", __kuid_val(curr->euid));
    }
    rcu_read_unlock();
}
