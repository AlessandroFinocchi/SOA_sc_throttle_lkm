#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "str_hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ingegnere Informatico");
MODULE_DESCRIPTION("Test di Hash Table concorrente RCU");

#define MODNAME "STR_HASH"

static struct string_hash hash_a;
static struct string_hash hash_b;

static int __init string_hash_test_init(void) {
    int ret;

    pr_info("STR_DEMO: ------------------------------------------");
    pr_info("STR_DEMO: Inizializzazione modulo hash table RCU.\n");


    ret = str_hash_init(&hash_a);
    if (ret) {
        pr_err("STR_DEMO: fallito inizializzare hash_a (%d)", ret);
        return ret;
    }

    ret = str_hash_init(&hash_b);
    if (ret) {
        pr_err("STR_DEMO: fallito inizializzare hash_b (%d)", ret);
        return ret;
    }

    /* Test di Inserimento */
    ret = str_hash_add(&hash_a, "sistema");
    if (ret) goto err;
    
    ret = str_hash_add(&hash_a, "operativo");
    if (ret) goto err;
    
    ret = str_hash_add(&hash_b, "avanzato");
    if (ret) goto err;

    /* Test di Ricerca */
    if (str_hash_lookup(&hash_a, "operativo"))
        pr_info("STR_DEMO: 'operativo' trovato (CORRETTO).\n");
    else
        pr_err("STR_DEMO Errore: 'operativo' non trovato (ERRORE).\n");

    if (!str_hash_lookup(&hash_a, "kernel"))
        pr_info("STR_DEMO: 'kernel' non trovato (CORRETTO).\n");
    else
        pr_err("STR_DEMO: 'kernel' trovato (ERRORE).\n");



    /* Test di Rimozione */
    str_hash_del(&hash_a, "sistema");
    
    if (!str_hash_lookup(&hash_a, "sistema"))
        pr_info("STR_DEMO: 'sistema' rimosso (CORRETTO).\n");
    else
        pr_err("STR_DEMO: 'sistema' non rimosso (ERRORE).\n");

    return 0;

err:
    pr_err("Errore di allocazione in fase di inizializzazione (%d).\n", ret);
    str_hash_cleanup(&hash_a);
    str_hash_cleanup(&hash_b);
    return ret;
}

static void __exit string_hash_test_exit(void)
{
    pr_info("STR_DEMO: Scaricamento modulo e deallocazione hash table...\n");
    str_hash_cleanup(&hash_a);
    str_hash_cleanup(&hash_b);
    pr_info("STR_DEMO: Modulo scaricato correttamente.\n");
}

module_init(string_hash_test_init);
module_exit(string_hash_test_exit);