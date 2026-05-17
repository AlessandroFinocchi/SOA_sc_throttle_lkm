# Sottosistema Token Bucket

## 1. Introduzione
Per effettuare il conteggio delle system call ritenute critiche viene utilizzato l'algoritmo token-bucket. Questo approccio è considerato lo stato dell'arte per i meccanismi di rate-limiting non distribuiti, considerando anche la sua semplicità. A supporto della logica si utilizza il LAPIC Timer, delegato al refill periodico dei token nel bucket.

## 2. Algoritmo Token-Bucket
L'implementazione è visibile nel sottomodulo `sctrt_tb.c`. La logica operativa si articola nei seguenti passaggi:
1. Inizializzazione del bucket con MAX numero di token.
2. Ogni $\tau$ secondi il bucket è ricaricato di tutti i token.
3. Quando arriva una richiesta, l'algoritmo controlla la disponibilità di un token

    - Se il bucket è non vuoto, si consuma un token e si consente la prosecuzione del flusso
    - Se il bucket è vuoto, il thread chiamante viene sospeso in attesa di prendere un token

Il timer scelto per calcolare la finestra temporale di $1$ secondo è quello della `struct timer_list`, un **Jiffies Timer** dal momento che non si è in un contesto hard-RT, caso in cui sarebbe stato imperativo l'utilizzo di un `hrtimer` per garantire una precisione delle scadenze nell'ordine dei nanosecondi. Di conseguenza, è importante aspettarsi un potenziale drift dell'ordine dei millisecondi, ma al tempo stesso la complessità algoritmica di inserimento e cancellazione è $O(1)$, rispetto all' $O(\log(N))$ nel caso degli `hrtimer`.

## 3. Dettagli implementativi
Per migliorare l'efficienza dell'algoritmo, utilizzato nell'interrupt context del handler della kprobe, l'implementazione si orienta verso l'utilizzo di operazioni lockless, in particolare usando *variabili atomiche*.

L'overhead della funzione per accedere ai token viene minimizzato rendendola *inline*, così da evitare le istruzioni di call e ret per la gestione dello stack. Inoltre, è usata la macro di branch prediction del kernel (`likely()`), poichè ci si aspetta che l'acquisizione del token abbia quasi sempre successo (tranne nei momenti di congestione).

La variabile atomica `tokens` viene allineata alla linea di cache, così da minimizzare le invalidazioni che essa può subire in cache. Un altro fenomeno che viene inoltre mitigato grazie all'allineamento è quello del **False Cache Sharing** con altre variabili non correlate scritte di frequente, per ridurre le invalidazioni a causa del protocollo di coerenza della cache.

La variabile atomica `tb_max_tokens`, la quale definisce il numero massimo di system call critiche eseguibili nella finestra temporale di $1$ secondo, viene configurata con l'attributo `__read_mostly` in modo da istruire il compilatore a posizionare la variabile nell'apposita sezione; questo permette a tali variabili di condividere le proprie linee di cache con altre variabili con le stesse caratteristiche, e dunque le corrispondenti linee di cache rimarranno sempre nello stato Shared e minimizzeranno i loro Cache miss.