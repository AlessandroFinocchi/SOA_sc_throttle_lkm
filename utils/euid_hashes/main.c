#include <linux/module.h>
#include <linux/kernel.h>
#include "euid_hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autore");
MODULE_DESCRIPTION("Dimostrazione Hash Table RCU per EUIDs con più tabelle");

#define MODNAME "HASH"

static struct euid_hash hash_a;
static struct euid_hash hash_b;

static int __init euid_demo_init(void)
{
    int ret;

    pr_info("EUID_DEMO: ------------------------------------------");
    pr_info("EUID_DEMO: Inizializzazione modulo e test hash table.");

    ret = euid_hash_init(&hash_a);
    if (ret) {
        pr_err("EUID_DEMO: fallito inizializzare hash_a (%d)", ret);
        return ret;
    }

    ret = euid_hash_init(&hash_b);
    if (ret) {
        pr_err("EUID_DEMO: fallito inizializzare hash_b (%d)", ret);
        return ret;
    }

    euid_hash_add(&hash_a, KUIDT_INIT(1000));
    euid_hash_add(&hash_a, KUIDT_INIT(1001));
    euid_hash_add(&hash_b, KUIDT_INIT(2000));

    if (euid_hash_lookup(&hash_a, KUIDT_INIT(1000)))
        pr_info("EUID_DEMO: hash_a trova 1000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a NON trova 1000 (comportamento errato).");

    if (!euid_hash_lookup(&hash_a, KUIDT_INIT(2000)))
        pr_info("EUID_DEMO: hash_a non trova 2000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a non trova 2000 (comportamento errato).");

    if (euid_hash_lookup(&hash_b, KUIDT_INIT(2000)))
        pr_info("EUID_DEMO: hash_b trova 2000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_b NON trova 2000 (comportamento errato).");

    euid_hash_del(&hash_a, KUIDT_INIT(1000));
    if (!euid_hash_lookup(&hash_a, KUIDT_INIT(1000)))
        pr_info("EUID_DEMO: hash_a conferma rimozione 1000 (comportamento atteso).");
    else
        pr_err("EUID_DEMO: hash_a NON conferma rimozione 1000 (comportamento errato).");


    pr_info("EUID_DEMO: Stampo hash_a:");
    euid_hash_print(&hash_a);
    pr_info("EUID_DEMO: Stampo hash_b:");
    euid_hash_print(&hash_b);

    return 0;
}

static void __exit euid_demo_exit(void)
{
    euid_hash_cleanup(&hash_a);
    euid_hash_cleanup(&hash_b);
    pr_info("EUID_DEMO: Modulo terminato e memoria deallocata.");
}

module_init(euid_demo_init);
module_exit(euid_demo_exit);