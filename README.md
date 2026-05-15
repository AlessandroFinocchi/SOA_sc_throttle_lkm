# System call Throttling LKM

## Specifica

Questa repository contiene un Linux Kernel Module (LKM) progettato per implementare un meccanismo di *system call throttling*. Il modulo espone un device driver che consente la registrazione e deregistrazione di:
- **Nomi di programmi** (eseguibili)
- **User ID** (effective)
- **Syscall numbers** (in accordo alla specifica x86_64)

I syscall numbers sono usati per tracciare system call considerate critiche in termini di scalabilità, performance o sicurezza.<br>

Ogni volta che una syscall registrata viene invocata da un programma o da un utente monitorato dal device driver, la sua effettiva esecuzione è subordinata al controllo di un **monitor** interno all'LKM.

### Funzionamento del Monitor

L'LKM può essere configurato con un parametro *MAX*, che rappresenta il numero massimo di invocazioni consentite per le syscall registrate all'interno di una **finestra temporale di 1 secondo**. Se il numero di chiamate all'interno di tale finestra supera la soglia *MAX* per un dato programma/utente monitorato, il thread chiamante viene sospeso temporaneamente, ritardandone o bloccandone l'esecuzione. Le system call monitorate possono essere di qualsiasi natura (bloccanti o non bloccanti).

Il monitor può essere **attivato o disattivato** dinamicamente: quando è disabilitato, non viene imposto alcun limite al tasso di invocazione delle syscall registrate.

### Sicurezza e Metriche

Il device driver garantisce che l'aggiornamento della configurazione (registrazione di utenti, programmi e syscall) sia consentito esclusivamente a thread in esecuzione con privilegi di amministratore (**EUID = 0**). La sola consultazione dello stato del monitor e degli elementi registrati è invece liberamente accessibile in qualsiasi momento.

Inoltre, l'LKM espone dei dati riguardanti:
- Il **massimo delay** applicato all'esecuzione di una syscall soggetta a throttling, indicando il relativo programma ed EUID.
- Il **numero massimo** e **medio** di thread che sono stati sospesi.

Fanno parte della repository anche il codice user-level necessario per la configurazione dell'LKM e gli applicativi per eseguire i test funzionali del sistema.

## Guida all'Uso
Per istruzioni sulla configurazione e l'utilizzo pratico, fare riferimento alla [Guida all'Uso](docs/usage_guide.md).

## Architettura
Per maggiori informazioni sul design del modulo, consultare la [Documentazione Architettura](docs/architecture.md).

## Test e User suite
Per informazioni sulla suite di programmi di test/user, consultare la [Documentazione test/user](docs/tests.md).

## Performance
Per vedere l'analisi delle performance effettuata 

## Sviluppi Futuri
- Il modulo è stato attualmente sviluppato per kernel 6.12.73. Tra i lavori futuri si prevede di estenderne la compatibilità per supportare in maniera nativa versioni aggiuntive del kernel Linux. Particolare attenzione va portata al modulo delle kprobe, che successivamente alla versione 6.12 ha subito dei cambiamenti che impediscono il riderezionamento del flusso in modo banale (vedere il componente [sctrt_hook](probes/sctrt_hook.c) e la [documentazione probes](docs/arch/probes.md)).