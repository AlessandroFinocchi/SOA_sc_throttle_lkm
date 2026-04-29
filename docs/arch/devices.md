# Sottosistema Devices

## 1. Introduzione
Il modulo implementa l'interfaccia di comunicazione tra lo user-space e il kernel-space avvalendosi di un dispositivo a caratteri (character device). Il sottosistema `devices` gestisce l'esposizione di tale nodo di dispositivo, al quale è delegata la ricezione di comandi di configurazione e il recupero dei dati di monitoraggio tramite chiamate `ioctl()`.

## 2. Struttura e Moduli Principali
L'architettura del sottosistema è rigorosamente bipartita per separare la gestione del ciclo di vita del dispositivo dall'implementazione logica dei comandi:

*   **`sctrt_dev.c` e `sctrt_dev.h`**: Costituiscono il nucleo della registrazione del dispositivo.
*   **`sctrt_dev_ioctl.c` e `sctrt_dev_ioctl.h`**: Isolano la logica di de-multiplexing dei comandi `ioctl`.

## 3. Nucleo del device
L'inizializzazione deel dispositivo, implementata in `sctrt_dev.c` e innescata dalla routine principale del modulo (`sctrt.c`), si articola nelle seguenti fasi formali:
1.  **Allocazione del Major Number**: Il kernel alloca dinamicamente una regione di identificatori per il character device, garantendo l'assenza di collisioni dello stesso.
2.  **Inizializzazione della struttura `cdev`**: La struttura dati interna del kernel per i dispositivi a caratteri viene allocata e associata al set di operazioni (`file_operations`).
3.  **Creazione del Device Node e Access Mode**: Attraverso le API del kernel (`class_create` e `device_create`), il nodo del dispositivo viene istanziato all'interno del virtual file system (tipicamente in `/dev/`). In questa fase è anche imposta l'*access mode*, limitando l'accesso in scrittura al solo utente `root` come da specifica.

## 4. Interfaccia ioctl
Il dispositivo funge da entry-point per operazioni che risultano fondamentali per la configurazione del *throttling* delle system call. Sulla base della suite di testing implementata (`tests/devices/`), il sottosistema garantisce la correttezza formale delle seguenti interazioni:

*   **Gestione dello Stato del Monitor (`test_ioctl_monitor_state.c`)**: Comandi atti ad attivare, disattivare o interrogare lo stato del monitor.
*   **Operazioni di Controllo (`test_ioctl_operations.c`)**: Interfacciamento diretto per la manipolazione delle regole di *throttling*, come la modifica limiti di frequenza, nonchè l'inserimento e rimozione delle system call, programmi e utenti ritenuti critici.
*   **Controllo degli Accessi (`test_ioctl_permissions.c`)**: Validazione rigorosa dei permessi. Garantisce che solo l'utente root possa accedere alle operazioni di scrittura per alterare la configurazione del monitor.
*   **Profilazione (`test_ioctl_profiling.c`)**: Estrazione delle metriche prestazionali e dei dati statistici raccolti dal sottosistema `profiler`.