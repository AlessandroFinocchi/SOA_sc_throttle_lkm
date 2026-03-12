#include <linux/module.h>
#include <linux/kernel.h>
#include "euid_hash.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autore");
MODULE_DESCRIPTION("Dimostrazione Hash Table RCU per EUIDs");

#define MODNAME "HASH"

static int __init euid_demo_init(void)
{
    kuid_t test_uid_1 = KUIDT_INIT(1000);
    kuid_t test_uid_2 = KUIDT_INIT(1001);
    int ret;

    pr_info("EUID_DEMO: Inizializzazione modulo e test hash table.\n");

    /* Test 1: Inserimento */
    ret = euid_hash_add(test_uid_1);
    if (ret) {
        pr_err("EUID_DEMO: Errore nell'allocazione per UID 1000\n");
        return ret;
    }
    euid_hash_add(test_uid_2);
    pr_info("EUID_DEMO: Inseriti EUID 1000 e 1001.\n");

    /* Test 2: Ricerca con RCU */
    if (euid_hash_lookup(test_uid_1))
        pr_info("EUID_DEMO: EUID 1000 trovato correttamente.\n");
    else
        pr_err("EUID_DEMO Errore: EUID 1000 mancante!\n");

    if (!euid_hash_lookup(KUIDT_INIT(9999)))
        pr_info("EUID_DEMO: EUID 9999 non trovato (comportamento atteso).\n");

    /* Test 3: Cancellazione */
    euid_hash_del(test_uid_1);
    pr_info("EUID_DEMO: EUID 1000 rimosso.\n");

    if (!euid_hash_lookup(test_uid_1))
        pr_info("EUID_DEMO: Conferma rimozione EUID 1000 con successo.\n");

    return 0;
}

static void __exit euid_demo_exit(void)
{
    /* Pulizia residua per evitare memory leak al ricaricamento del modulo */
    euid_hash_del(KUIDT_INIT(1001));
    euid_hash_cleanup();
    
    pr_info("EUID_DEMO: Modulo terminato e memoria deallocata.\n");
}

module_init(euid_demo_init);
module_exit(euid_demo_exit);