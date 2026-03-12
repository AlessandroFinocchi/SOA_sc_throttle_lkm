#include <linux/module.h>
#include <linux/hashtable.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/jhash.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "str_hash.h"

#define STR_HASH_BITS 8

static DEFINE_SPINLOCK(string_hash_lock); // Spinlock per writer RCU
static DEFINE_HASHTABLE(string_hash_table, STR_HASH_BITS);

struct string_node {
    struct hlist_node node;
    struct rcu_head rcu;
    char str[];
};

static inline u32 hash_string(char *str) {
    return jhash(str, strlen(str), 0);
}

int str_hash_add(char *new_str) {
    struct string_node *new_node;
    u32 hash;
    size_t len;

    if (!new_str)
        return -EINVAL;

    len = strlen(new_str) + 1;
    // Allocazione in process ctx., usa GFP_ATOMIC se in interrupt ctx.
    new_node = kmalloc(sizeof(*new_node) + len, GFP_KERNEL); 
    if (!new_node)
        return -ENOMEM;

    strscpy(new_node->str, new_str, len);
    hash = hash_string(new_str);

    spin_lock(&string_hash_lock);
    hash_add_rcu(string_hash_table, &new_node->node, hash);
    spin_unlock(&string_hash_lock);

    return 0;
}

bool str_hash_lookup(char *target) {
    struct string_node *curr;
    u32 hash;
    bool found = false;

    if (!target)
        return false;

    hash = hash_string(target);

    rcu_read_lock();
    hash_for_each_possible_rcu(string_hash_table, curr, node, hash) {
        if (strcmp(curr->str, target) == 0) {
            found = true;
            break;
        }
    }
    rcu_read_unlock();

    return found;
}

void str_hash_del(char *target) {
    struct string_node *curr;
    struct hlist_node *tmp;
    u32 hash;

    if (!target)
        return;

    hash = hash_string(target);

    spin_lock(&string_hash_lock);
    hash_for_each_possible_safe(string_hash_table, curr, tmp, node, hash) {
        if (strcmp(curr->str, target) == 0) {
            hash_del_rcu(&curr->node);
            spin_unlock(&string_hash_lock);
            kfree_rcu(curr, rcu);
            return;
        }
    }
    spin_unlock(&string_hash_lock);
}

void str_hash_cleanup(void) {
    struct string_node *curr;
    struct hlist_node *tmp;
    int bkt;

    spin_lock(&string_hash_lock);
    /* L'iteratore _safe è essenziale per non corrompere la lista durante le rimozioni */
    hash_for_each_safe(string_hash_table, bkt, tmp, curr, node) {
        hash_del_rcu(&curr->node);
        kfree_rcu(curr, rcu);
    }
    spin_unlock(&string_hash_lock);

    /* * RCU barrier blocca l'esecuzione finché tutte le callback di kfree_rcu 
     * pendenti non sono state completate. Cruciale prima di scaricare il modulo.
     */
    rcu_barrier();
}