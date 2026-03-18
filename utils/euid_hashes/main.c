#include <linux/module.h>
#include <linux/kernel.h>
#include "euid_hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autore");
MODULE_DESCRIPTION("Dimostrazione Hash Table RCU per EUIDs con più tabelle");

#define MODNAME "HASH"

struct data {
    struct euid_hash *hash_a;
    int x;
};

struct data *d;

static int __init euid_demo_init(void)
{
    int ret;

    pr_info("EUID_DEMO: ------------------------------------------");
    pr_info("EUID_DEMO: Inizializzazione modulo e test hash table.");

    d = kmalloc(sizeof(struct data), GFP_KERNEL);
    if(!d)
        return -ENOMEM;

    d->hash_a = kmalloc(sizeof(struct euid_hash), GFP_KERNEL);
    if(!d->hash_a)
        return -ENOMEM;

    ret = euid_hash_init(d->hash_a);
    if (ret) {
        pr_err("EUID_DEMO: fallito inizializzare hash_a (%d)", ret);
        return ret;
    }

    euid_hash_add(d->hash_a, KUIDT_INIT(1000));

    if (euid_hash_lookup(d->hash_a, KUIDT_INIT(1000)))
        pr_info("EUID_DEMO: hash_a trova 1000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a NON trova 1000 (comportamento errato).");

    if (!euid_hash_lookup(d->hash_a, KUIDT_INIT(2000)))
        pr_info("EUID_DEMO: hash_a non trova 2000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a non trova 2000 (comportamento errato).");

    euid_hash_del(d->hash_a, KUIDT_INIT(1000));
    if (!euid_hash_lookup(d->hash_a, KUIDT_INIT(1000)))
        pr_info("EUID_DEMO: hash_a conferma rimozione 1000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a NON conferma rimozione 1000 (comportamento errato).");


    pr_info("EUID_DEMO: Stampo hash_a:");
    euid_hash_print(d->hash_a);

    return 0;
}

static void __exit euid_demo_exit(void)
{
    euid_hash_cleanup(d->hash_a);
    kfree(d->hash_a);
    kfree(d);
    pr_info("EUID_DEMO: Modulo terminato e memoria deallocata.");
}

module_init(euid_demo_init);
module_exit(euid_demo_exit);