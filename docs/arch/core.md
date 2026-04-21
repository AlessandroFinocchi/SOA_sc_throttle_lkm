# Core

## Meccanismo di rate-limiting per il throttling
Per effettuare il conteggio delle system call ritenute critiche viene utilizzato l'algoritmo token-bucket, considerato lo stato dell'arte per meccanismi di rate-limiting non distribuiti vista la sua semplicità d'azione. A sostegno dell'algoritmo viene in aiuto il LAPIC Timer, usato per il refill dei token nel bucket

## Algoritmo Token-Bucket
L'implementazione è visibile nel sotto-modulo `sctrt_tb.c`. L'algoritmo funziona nel seguente modo:
1. Inizializzazione del bucket con MAX numero di token.
2. Ogni $\tau$ secondi il bucket è ricaricato di tutti i token.
3. Quando arriva una richiesta, l'algoritmo controlla se c'è un token
    
    - Se il bucket è non vuoto, si consuma un token e si prosegue
    - Se il bucket è vuoto, il thread chiamante viene bloccato

Il timer scelto per calcolare la finestra temporale di $1$ secondo è quello della `struct timer_list`, un **Jiffies Timer** dal momento che non si è in un contesto hard-RT, caso in cui sarebbe stato più appropriato l'utilizzo di un `hrtimer` che assicura una precisione del nanosecondo. E' quindi importante notare che un eventuale drift dell'ordine dei millisecondi sarà presente.