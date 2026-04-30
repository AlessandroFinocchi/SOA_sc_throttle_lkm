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
Per migliorare l'efficienza dell'algoritmo, l'implementazion si orienta verso l'utilizzo di operaizoni lockless, in particolare tramite *variabili atomiche* gestite tramite operazioni *RMW*.

L'overhead della funzione per accedere ai token viene minimizzato rendendola *inline*, così da evitare le istruzioni di call e ret per la gestione dello stack. Inoltre, è le macro di branch prediction del kernel (`likely()`), poichè ci si aspetta che 
l'acquisizione del token abbia quasi sempre successo (tranne nei momenti di congestione).

Se la variabile atomica `tokens` condividesse la cache line con altre variabili non correlate scritte di frequente, subirebbe invalidazioni costanti a causa del protocollo di coerenza della cache, causando il fenomeno del **False Cache Sharing**. Dunque, per essa si forza l'allineamento alla linea di cache affinché occupi una cache line esclusiva.

La gestione della variabile atomica `tb_max_tokens`, la quale definisce il numero massimo di system call critiche eseguibili nella finestra temporale di $1$ secondo, viene configurata con l'attributo `__read_mostly` in modo da istruire il compilatore a posizionare la variabile nell'apposita sezione, così da aspettarsi che le linee di cache in cui queste variabili sono caricate siano separate. Questo è ragionevole per dati che sono scritti raramente, e acceduti spesso.