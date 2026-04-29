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

Il timer scelto per calcolare la finestra temporale di $1$ secondo è quello della `struct timer_list`, un **Jiffies Timer** dal momento che non si è in un contesto hard-RT, caso in cui sarebbe stato imperativo l'utilizzo di un `hrtimer` per garantire una precisione delle scadenze nell'ordine dei nanosecondi. Di conseguenza, è importante aspettarsi un potenziale drift dell'ordine dei millisecondi.