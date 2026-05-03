/* Algoritmo Token Bucket per il rate-limiting del throttling */
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/cache.h>
#include <linux/atomic.h>

#include "sctrt.h"
#include "sctrt_tb.h"
#include "sctrt_core.h"
#include "sctrt_profiler.h"


/** 
 * Variabile atomica per il conteggio dei token. 
 * Assicura l'assenza di data race senza l'ausilio di lock bloccante.
 * Alloca la variabile su un indirizzo di memoria allineato alla dimensione
 * della cache line specifica dell'architettura hardware in uso.
 */
atomic_t tb_tokens ____cacheline_aligned_in_smp;

/**
 * Configurazione dei parametri del Token Bucket. L'annotazione __read_mostly 
 * posiziona la variabile in una sezione della memoria dedicata ai dati 
 * raramente modificati, evitando invalidazioni superflue della cache.
 */
static atomic_t tb_max_tokens __read_mostly;

/**
 * Strutture per la gestione del Timer standard */
static struct timer_list tb_timer;

/**
 * tb_timer_callback() - Handler del timer.
 * @t: puntatore alla struttura timer_list.
 *
 * Viene eseguito in contesto SoftIRQ. Effettua il refill atomico e riarma
 * se stesso per il tick corrispondente a 1 secondo nel futuro.
 */
static void tb_timer_callback(struct timer_list *t) {
    /* Refill dei token */
    atomic_set(&tb_tokens, atomic_read(&tb_max_tokens));

    sctrt_profiler_sample_window();
    
    /** Riarmo del timer:
     * jiffies rappresenta il tempo corrente.
     * HZ è una costante architetturale (1000) che rappresenta
     * il numero di tick in esattamente 1 secondo.
     */
    mod_timer(&tb_timer, jiffies + 1 * HZ);

    // printk("%s: token-bucket - Token refill\n", MODNAME);

    sctrt_wake_up_weq();
}

/**
 * token_bucket_update_max() - Aggiorna la capacità del bucket a runtime.
 * @new_max_tokens: il nuovo numero massimo di token al secondo.
 * L'aggiornamento viene registrato tramite operazione atomica. 
 * Il nuovo limite sarà caricato dal timer_callback nel prossimo ciclo.
 */
void token_bucket_set_max(uint new_max) {
    if (new_max == 0) {
        printk("%s: token-bucket - Attempted to set max_tokens to 0\n", MODNAME);
        return;
    }
    
    atomic_set(&tb_max_tokens, new_max);
    printk("%s: token-bucket - MAX tokens updated to %u\n", MODNAME, new_max);

    sctrt_wake_up_weq();
}

uint token_bucket_get_max(void) {
    return atomic_read(&tb_max_tokens);
}

int token_bucket_init(uint max_tokens) {
    if (max_tokens == 0)
        return -EINVAL;

    atomic_set(&tb_max_tokens, max_tokens);
    atomic_set(&tb_tokens, max_tokens);
    
    /*
     * timer_setup() collega la struttura timer_list alla funzione di callback.
     * Il terzo parametro è riservato a flag opzionali (es. TIMER_DEFERRABLE),
     * che qui omettiamo passando 0.
     */
    timer_setup(&tb_timer, tb_timer_callback, 0);
    
    /* Avvia il timer schedulandolo tra esattamente 1 secondo (HZ tick) */
    mod_timer(&tb_timer, jiffies + 1 * HZ);
    
    printk("%s: token-bucket - Initialized with MAX=%d and \xcf\x84 = 1s\n", MODNAME, max_tokens);
    return 0;
}

void token_bucket_exit(void) {
    /*
     * del_timer_sync() è imperativo al posto di del_timer() in contesti SMP.
     * Garantisce non solo la cancellazione del timer, ma blocca l'esecuzione
     * finché l'handler non è terminato nel caso stia venendo servito da un
     * SoftIRQ concorrente su un'altra CPU. Evita kernel panic durante l'unload.
     */
    timer_delete_sync(&tb_timer);
    printk("%s: token-bucket - Terminated and exited\n", MODNAME);
}