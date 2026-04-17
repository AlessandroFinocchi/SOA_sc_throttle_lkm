# KProbes

## Motivazione
Le Kprobes permettono di implementare instrumentazione dinamica all'interno di un LKM: di fatto quello che si fa è inserire un breakpoint all'istruzione che precede la chiamata di una qualsiasi syscall, e quando questa viene invocata, il codice della kprobe viene eseguita. Sia montaggio che smontaggio sono operazioni dinamiche e asincrone che il sottomodulo gestisce internamente in maniera autonoma, assicurando una gestione semplice all'interno dell'LKM.

## Ricerca del contesto di probing
(Da rivedere) La ricerca del contesto di probing funziona grazie del design dell'allocatore della memoria per-CPU (mm/percpu.c), che divide le variabili in base al loro ciclo di vita: *Statiche* vs *Dinamiche*.
- Variabili Statiche (Kernel Core): Le kprobes sono un sottosistema integrato (built-in) nel kernel. La variabile che mantiene lo stato (es. current_kprobe) è definita tramite la macro DEFINE_PER_CPU nei sorgenti del kernel stesso (kernel/kprobes.c). Durante la fase di linking del file immagine vmlinux, queste variabili vengono compattate nel primissimo blocco di memoria per-CPU, noto come il First Chunk. I loro offset relativi alla base per-CPU partono da $0$ e crescono progressivamente di poche decine di kilobyte.
- Variabili Dinamiche (Loadable Kernel Modules):Il tuo modulo (throttler_module) viene caricato a run-time tramite la syscall init_module (innescata da insmod). Durante questa fase, il modulo loader deve trovare spazio per la tua per_cpu_var. Poiché il First Chunk statico è ormai sigillato, il kernel richiede un nuovo blocco di memoria all'allocatore per-CPU tramite funzioni come pcpu_alloc().

Questo meccanismo impone la seguente relazione spaziale invariante (in condizioni standard):$$\text{Offset}_{\text{Dinamico}} > \text{Offset}_{\text{Statico}}$$