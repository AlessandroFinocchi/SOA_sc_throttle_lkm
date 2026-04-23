#ifndef _SCTRT_TB_H
#define _SCTRT_TB_H

#include <linux/types.h>

extern atomic_t tb_tokens;

int token_bucket_init(uint max_tokens);
void token_bucket_exit(void);
void token_bucket_set_max(uint new_max);
uint token_bucket_get_max(void);

/**
 * take_token() - Tenta di prelevare un token dal bucket.
 * * Ritorna: true se il token è stato acquisito con successo, false altrimenti.
 * Questa funzione è progettata per essere chiamata nel critical path,
 * ottimizzando il tempo di esecuzione tramite primitive lock-free.
 * __always_inline forza l'inlining ignorando le euristiche del compilatore.
 */
static __always_inline bool take_token(void) {
    /*
     * atomic_dec_if_positive() esegue un test-and-decrement in un'unica 
     * operazione atomica (spesso implementata via CMPXCHG a livello assembly).
     * Sottrae 1 solo se il valore attuale è strettamente maggiore di 0.
     * Questo evita i tipici colli di bottiglia causati dalla lock contention.
     */
    return likely(atomic_dec_if_positive(&tb_tokens) >= 0);
}

#endif