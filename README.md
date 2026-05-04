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

## Architettura
Per maggiori informazioni sul design del modulo, consultare la [Documentazione Architettura](docs/architecture.md).

## Guida all'Uso
Per istruzioni sulla configurazione e l'utilizzo pratico, fare riferimento alla [Guida all'Uso](docs/usage_guide.md).

## Sviluppi Futuri
- Il modulo è stato attualmente sviluppato per kernel 6.17. Tra i lavori futuri si prevede di estenderne la compatibilità per supportare in maniera nativa versioni aggiuntive del kernel Linux.
- Il componente di intercettazione `sctrt_hook` può ridirezionare un thread svegliato da un segnale che non ha preso alcun token verso la ni syscall, attualmente un tale thread viene semplicemente rimesso a dormire.