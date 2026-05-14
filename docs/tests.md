# Test suite

## Device
I test sul device sono presenti nella directory `SOA_sc_throttle_lkm/tests/devices/`. Vanno eseguiti a monte dell'inserimento del modulo. Di seguito una spiegazione dettagliata di ciascun test implementato.

### perm
Questo test va eseguito con `EUID = 0`. Effettua i seguenti controlli:
1. Un utente root possa fare tutte le operzioni
2. Un utente non-root possa effettuare solo le operazoni di lettura e on quelle di scrittura

### ops
Questo test va eseguito con `EUID = 0`. Controlla che le seguenti operazioni vadano a buon fine:
1. Accensione del monitor
2. Impostazione del rate-limit di system call
3. Registrazione di system calls
4. Registrazione di utenti
5. Registrazione di nomi di programmi

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

### perf
Questo test misura le performance del monitor.