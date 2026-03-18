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

int euid_hash_init(struct euid_hash **hash) {
    if (!hash)
        return -EINVAL;

    if(!(*hash = kzalloc(sizeof(struct euid_hash), GFP_KERNEL))) {
        return -ENOMEM;
    }

    spin_lock_init(&(*hash)->lock);
    hash_init((*hash)->table);
    return 0;
}

int euid_hash_add(struct euid_hash *hash, kuid_t euid) {
    struct euid_node *new_node;
    u32 h;

    if (!hash)
        return -EINVAL;

    if (euid_hash_lookup(hash, euid))
        return -EEXIST

    new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
    if (!new_node)
        return -ENOMEM;

    new_node->euid = euid;
    h = hash_euid(euid);

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

int euid_hash_del(struct euid_hash *hash, kuid_t euid) {
    struct euid_node *curr;
    u32 h;

    if (!hash)
        return -EINVAL;

    h = hash_euid(euid);

    // I lettori (in interrupt ctx) non acquisiscono mai lo spinlock,
    // quindi non serve spin_lock_irqsave()
    spin_lock(&hash->lock);
    hash_for_each_possible(hash->table, curr, node, h) {
        if (uid_eq(curr->euid, euid)) {
            hlist_del_rcu(&curr->node);
            spin_unlock(&hash->lock);
            kfree_rcu(curr, rcu);
            return 0;
        }
    }
    spin_unlock(&hash->lock);

    return -EINVAL;
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

    kfree(hash);
}

void euid_hash_print(struct euid_hash *hash) {
    struct euid_node *curr;
    int bkt;

    if (!hash)
        return;

    rcu_read_lock();
    pr_info("EUID_HASH: EUID registrati:");
    hash_for_each_rcu(hash->table, bkt, curr, node) {
        pr_cont(" %u", __kuid_val(curr->euid));
    }
    pr_cont("\n");
    rcu_read_unlock();
}
