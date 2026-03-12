#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "str_hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ingegnere Informatico");
MODULE_DESCRIPTION("Test di Hash Table concorrente RCU");

#define MODNAME "STR_HASH"

static int __init string_hash_test_init(void)
{
    int ret;

    pr_info("STR_DEMO: Inizializzazione modulo hash table RCU.\n");

    /* Test di Inserimento */
    ret = str_hash_add("sistema");
    if (ret) goto err;
    
    ret = str_hash_add("operativo");
    if (ret) goto err;
    
    ret = str_hash_add("avanzato");
    if (ret) goto err;

    /* Test di Ricerca */
    if (str_hash_lookup("operativo"))
        pr_info("STR_DEMO: 'operativo' trovato in O(1).\n");
    else
        pr_err("STR_DEMO Errore: 'operativo' non trovato.\n");

    if (!str_hash_lookup("kernel"))
        pr_info("STR_DEMO: 'kernel' correttamente non trovato.\n");

    /* Test di Rimozione */
    str_hash_del("sistema");
    
    if (!str_hash_lookup("sistema"))
        pr_info("STR_DEMO: 'sistema' rimosso e invisibile ai lettori.\n");

    return 0;

err:
    pr_err("Errore di allocazione in fase di inizializzazione (%d).\n", ret);
    str_hash_cleanup();
    return ret;
}

static void __exit string_hash_test_exit(void)
{
    pr_info("STR_DEMO: Scaricamento modulo e deallocazione hash table...\n");
    str_hash_cleanup();
    pr_info("STR_DEMO: Modulo scaricato correttamente.\n");
}

module_init(string_hash_test_init);
module_exit(string_hash_test_exit);