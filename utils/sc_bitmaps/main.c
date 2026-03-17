#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "sc_bitmap.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ingegnere Informatico");
MODULE_DESCRIPTION("Test di Bitmap");

#define MODNAME "SC_BITMAP"

static struct sc_bitmap *bitm1;
static struct sc_bitmap *bitm2;

static int __init sc_bitmap_test_init(void) {
    int ret;

    pr_info("BITM_DEMO: ------------------------------------------");
    pr_info("BITM_DEMO: Inizializzazione modulo sc_bitmap.\n");

    ret = sc_bitmap_create(&bitm1, 256u);
    if (ret) {
        pr_err("BITM_DEMO: fallito inizializzare bitm1 (%d)", ret);
        return ret;
    }

    pr_info("BITM_DEMO: allocata bitm1.\n");
    pr_info("BITM_DEMO: test %p.\n", bitm1);
    pr_info("BITM_DEMO: test %p.\n", bitm1->bitmap);

    ret = sc_bitmap_create(&bitm2, 256u);
    if (ret) {
        pr_err("BITM_DEMO: fallito inizializzare bitm2 (%d)", ret);
        return ret;
    }

    pr_info("BITM_DEMO: allocata bitm2.\n");
    pr_info("BITM_DEMO: test %p.\n", bitm2);
    pr_info("BITM_DEMO: test %p.\n", bitm2->bitmap);

    /* Test di Inserimento */
    ret = sc_bitmap_register(bitm1, 1u);
    if (ret) goto err;
    
    ret = sc_bitmap_register(bitm1, 2u);
    if (ret) goto err;
    
    ret = sc_bitmap_register(bitm2, 3u);
    if (ret) goto err;

    pr_info("BITM_DEMO: registrazioni ok.\n");

    /* Test di Ricerca */
    if (sc_bitmap_lookup(bitm1, 1))
        pr_info("BITM_DEMO: 1 trovato in bitm1 (CORRETTO).\n");
    else
        pr_err("BITM_DEMO Errore: 1 non trovato in bitm1 (ERRORE).\n");

    if (!sc_bitmap_lookup(bitm2, 1))
        pr_info("BITM_DEMO: 1 non trovato in bitm2 (CORRETTO).\n");
    else
        pr_err("BITM_DEMO: 1 trovato in bitm2 (ERRORE).\n");


    /* Test di Rimozione */
    sc_bitmap_unregister(bitm1, 1);
    
    if (!sc_bitmap_lookup(bitm1, 1))
        pr_info("BITM_DEMO: 1 rimosso (CORRETTO).\n");
    else
        pr_err("BITM_DEMO: 1 non rimosso (ERRORE).\n");

    return 0;

err:
    pr_err("Errore di allocazione in fase di inizializzazione (%d).\n", ret);
    sc_bitmap_cleanup(bitm1);
    sc_bitmap_cleanup(bitm2);
    return ret;
}

static void __exit sc_bitmap_test_exit(void)
{
    pr_info("BITM_DEMO: Scaricamento modulo e deallocazione bitmap...\n");
    sc_bitmap_cleanup(bitm1);
    sc_bitmap_cleanup(bitm2);
    pr_info("BITM_DEMO: Modulo scaricato correttamente.\n");
}

module_init(sc_bitmap_test_init);
module_exit(sc_bitmap_test_exit);