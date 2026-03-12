#include <linux/module.h>
#include <linux/hashtable.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "euid_hash.h"

#define EUID_HASH_BITS 8

static DEFINE_SPINLOCK(euid_hash_lock); // Spinlock per writer RCU
static DEFINE_HASHTABLE(euid_hash_table, EUID_HASH_BITS);

struct euid_node {
    struct hlist_node node;
    struct rcu_head rcu;
    kuid_t euid;
};

static inline u32 hash_euid(kuid_t euid) {
    return __kuid_val(euid);
}

int euid_hash_add(kuid_t euid) {
    struct euid_node *new_node;
    u32 hash;

    // Allocazione in process ctx., usa GFP_ATOMIC se in interrupt ctx.
    new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);  
    if (!new_node)
        return -ENOMEM;

    new_node->euid = euid;
    hash = hash_euid(euid);

    spin_lock(&euid_hash_lock);
    hash_add_rcu(euid_hash_table, &new_node->node, hash);
    spin_unlock(&euid_hash_lock);
    
    return 0;
}

bool euid_hash_lookup(kuid_t euid) {
    struct euid_node *curr;
    u32 hash;
    bool found = false;

    hash = hash_euid(euid);

    rcu_read_lock();
    hash_for_each_possible_rcu(euid_hash_table, curr, node, hash) {
        if (uid_eq(curr->euid, euid)) {
            found = true;
            break;
        }
    }
    rcu_read_unlock();
    
    return found;
}

void euid_hash_del(kuid_t euid) {
    struct euid_node *curr;
    struct hlist_node *tmp;
    u32 hash;

    hash = hash_euid(euid);

    spin_lock(&euid_hash_lock);
    hash_for_each_possible_safe(euid_hash_table, curr, tmp, node, hash) {
        if (uid_eq(curr->euid, euid)) {
            hash_del_rcu(&curr->node);
            spin_unlock(&euid_hash_lock);
            kfree_rcu(curr, rcu);
            return;
        }
    }
    spin_unlock(&euid_hash_lock);
}

void euid_hash_cleanup(void) {
    struct euid_node *curr;
    struct hlist_node *tmp;
    int bkt;

    spin_lock(&euid_hash_lock);
    /* L'iteratore _safe è essenziale per non corrompere la lista durante le rimozioni */
    hash_for_each_safe(euid_hash_table, bkt, tmp, curr, node) {
        hash_del_rcu(&curr->node);
        kfree_rcu(curr, rcu);
    }
    spin_unlock(&euid_hash_lock);

    /* * RCU barrier blocca l'esecuzione finché tutte le callback di kfree_rcu 
     * pendenti non sono state completate. Cruciale prima di scaricare il modulo.
     */
    rcu_barrier();
}