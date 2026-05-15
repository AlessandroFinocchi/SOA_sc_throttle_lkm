# Test/User suite

## Device
I test e i programmi user sul device sono presenti nella directory `SOA_sc_throttle_lkm/tests/devices/`. Vanno eseguiti a monte dell'inserimento del modulo. Di seguito una spiegazione dettagliata di ciascun programma implementato.

### perm
Questo test va eseguito con `EUID = 0`. Effettua i seguenti controlli:
1. Un utente root possa fare tutte le operzioni
2. Un utente non-root possa effettuare solo le operazoni di lettura e on quelle di scrittura

### ops
Questo test va eseguito con `EUID = 0`. Controlla che le seguenti operazioni vadano a buon fine:
1. Accensione del monitor
2. Impostazione del rate-limit di system call (valore 2)
3. Registrazione di system calls (getpid)
4. Registrazione di utenti (1000 e 1001)
5. Registrazione di nomi di programmi ()

### state
Questo test va eseguito con `EUID = 0`. Controlla le operazioni di accensione e spegnimento del monitor.
1. Per l'accensione del monitor eseguire:
    ```sh
    sudo ./state 1
    ```
2. Per lo spegnimento del monitor eseguire:
    ```sh
    sudo ./state 0
    ```

### prof
Questo test permette la visualizzazione delle informazioni di profilazione sulle metriche di performance del monitor. Eseguito parallelamente ad uno degli stress test (vedere sotto-sezione **stress tests** della sezione **Probes** di questo documento), per ottenere le informazioni in tempo reale.

## Probes
I test e i programmi user sulle kernel probes sono presenti nella directory `SOA_sc_throttle_lkm/tests/probes/`. Vanno eseguiti a monte dell'inserimento del modulo. Di seguito una spiegazione dettagliata di ciascun programma implementato.

### single
Questo programma esegue una singola chiamata alla system call `getpid`.

### stress tests
Sono implementati 4 stress test:
- `stress`: un thread chiama una `getpid` ogni 50 millisecondi, dunque circa 20 al secondo.
- `stress2`: un thread chiama una `getpid` ogni 50 millisecondi, dunque circa 20 al secondo, e un altro thread gli invia costantemente dei segnali `SIGALRM` in modo che, se messo a dormire in una wait-event queue interrompibile, si risvegli e continui l'esecuzione.
- `stresses`: versione di `stress` multi-threaded.
- `stresses2`: versione di `stress2` multi-threaded.

### perf
Questo programma va eseguito con `EUID = 0`. Infatti, per minimizzare la varianza delle statistiche calcolate, il programma imposta la sua affinità su una CPU e la sua politica di scheduling come FIFO per minimizzare la sua preemption. Così facendo, è in grado di calcolare le performance di chiamate a system call da parte di un programma non registrato per essere soggetto al monitoraggio implementato. Quindi, eseguendo il programma con e senza l'lkm montato, questo aiuta a capire il rallentamento effettivo portato a tutti quei programmi che non sono soggetti al monitoraggio delle loro system call critiche.