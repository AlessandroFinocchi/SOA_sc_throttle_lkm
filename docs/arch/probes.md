# Sottosistema KProbes

## 1. Introduzione
Il sottosistema `probes` costituisce il nucleo operativo per l'intercettazione delle system call.
Tramite le **Kprobes** l'LKM può instrumentare dinamicamente il codice delle system call del kernel, e sia montaggio che smontaggio sono operazioni dinamiche e asincrone che il sottomodulo gestisce internamente in maniera autonoma, assicurando una gestione semplice.

## 2. Componenti Architetturali e Moduli
L'infrastruttura di intercettazione è modulata in due componenti principali, separati per garantire coesione logica e isolamento delle responsabilità:

*   **`sctrt_hook.c` e `sctrt_hook.h`**: Questo modulo implementa la logica di registrazione e deregistrazione dinamica dell'handler di intercettazione delle system call.
*   **`sctrt_kprobectx_saver.c` e `sctrt_kprobectx_saver.h`**: Permette il salvataggio del contesto di probing, necessario per rendere la kprobe preemptable.

## 3. Ricerca del contesto di probing
La ricerca del contesto di probing è funzione del design dell'allocatore della memoria per-CPU (`linux/percpu.c`), che divide le variabili in base al loro ciclo di vita: *Statiche* vs *Dinamiche*.
- **Variabili Statiche**: Le kprobes sono un sottosistema integrato nel kernel. La variabile che mantiene lo stato è definita tramite la macro `DEFINE_PER_CPU` nei sorgenti del kernel stesso (`linux/kprobes.c`). Durante la fase di linking, queste variabili vengono compattate nel primissimo blocco di memoria per-CPU, noto come *First Chunk*. I loro offset relativi alla base per-CPU partono da $0$ e crescono progressivamente di poche decine di kilobyte, occupando parzialmente il First Chunk.
- **Variabili Dinamiche**: Tutte le variabili allocate tramite `alloc_percpu()` sono caricate a run-time dall'allocatore in aree successive a quelle occupate dalle variabili per-CPU statiche.

Questo meccanismo impone la seguente relazione spaziale invariante (in condizioni standard):$$\text{Offset}_{\text{Dinamico}} > \text{Offset}_{\text{Statico}}$$