# Sottosistema KProbes

## 1. Introduzione
Il sottosistema `probes` costituisce il nucleo operativo per l'intercettazione delle system call.
Tramite le **Kprobes** l'LKM può instrumentare dinamicamente il codice delle system call del kernel, e sia montaggio che smontaggio sono operazioni dinamiche e asincrone che il sottomodulo gestisce internamente in maniera autonoma, assicurando una gestione semplice.

## 2. Componenti Architetturali e Moduli
L'infrastruttura di intercettazione è modulata in due componenti principali, separati per isolare le responsabilità operative:

*   **`sctrt_hook.c` e `sctrt_hook.h`**: Questo sottomodulo implementa la logica di registrazione e deregistrazione dinamica dell'handler di intercettazione delle system call.
*   **`sctrt_kprobectx_saver.c` e `sctrt_kprobectx_saver.h`**: Permette il salvataggio del contesto di probing, necessario per rendere la kprobe preemptable.

## 3. Ricerca del contesto di probing
Avere a disposizioe il contesto di probing è essenziale per rendere una kprobe preemptable. Infatti, normalmente gli handler delle kprobe eseguono in interrupt context, ma in questo caso, siccome l'handler della kprobe può mettere a dormire il thread che lo invoca su una wait-event queue, serve creare una finestra bloccante interna all'handler per permette questa operazione. 

Per farlo, è cruciale avere a disposizione il contesto di probing, in modo da ripristinarlo correttamente una volta che la sezione bloccante temina.

Nella ricerca del conteso di probing, quello che si cerca in particolare è l'offset della struttura di contesto di probing `struct kprobe`, presente quando una kprobe esegue, e posizionata sempre allo stesso offset in tutte le per-CPU memory (per via di come funziona il sottosistema `percpu`). Dunque, una volta trovato, può essere salvato per essere riutilizzato.

A questo punto:
- Prima di entrare nella sezione bloccante, si mette a `NULL` l'area di memoria in cui è presente la struttura per simulare di non eseguire alcuna probe.
- Dopo essere usciti dalla sezione bloccante, si ripristina il contesto di probe in modo che il sottosistema di probing si comporti correttamente mentre esegue l'handler.


## 4. Hijacking del flusso d'esecuzione
Per robustezza del modulo quando è configurato con wait-event queue interrompibili, se un thread è svegliato da un segnale e non perchè ha preso un token, per prevenire situazioni in cui un thread appositamente fa sì di essere colpito da segnali per aggirare il rate-liming del throttling, si annulla l'effetto della system call tramite il seguente meccanismo di *Hijacking del flusso d'esecuzione*:

```c
unsigned long ret_addr = *(unsigned long *)the_regs->sp;
the_regs->ip = ret_addr;
the_regs->sp += sizeof(long);
the_regs->ax = -EPERM;
return 1;
```  

Questo codice:
1. Estrae l'indirizzo dell'istruzione successiva alla system call invocata e lo mette in `ret_addr`.
2. Sovrascrive l'instruction pointer con il valore di `ret_addr` in modo da continuare l'esecuzione da quel punto.
3. Riallinea lo stack, simulando l'effetto di una `pop`.
4. Imposta il valore di ritorno della system call a `-EPERM`, comunicando che l'operazione è stata annullata poichè non permessa.
5. Ritorna 1: ritornare un valore diverso da 0 in una kprobe ha come effetto quello di annullare il **Single-Stepping** dell'istruzione originale, andando di fatto a ripristinare immediatamente i registri in funzione della `struct pt_regs` modificata e poi riprendendo il Single-Stepping.


## 5. Simbolo della probe
Il simbolo che la kernel probe intercetta per intercettare una system cal è `x64_sys_call`: questo è l'unico simbolo, nel flusso d'esecuzione di una system call, al quale è possibile agganciare una kprobe, dunque è stata una scelta obbligata in questo caso.

La catena di chiamate per arrivare ad una system call infatti è la seguente

$$
\text{(asm) entry\_SYSCALL\_64}
\rightarrow \text{(c) do\_syscall\_64}
\rightarrow \text{(c) do\_syscall\_x64}
\rightarrow \text{(c) x64\_sys\_call}
$$

E l'unico simbolo instrumentabile è proprio l'ultimo.

Per quanto riguarda la gestione dei segnali dunque, in caso di hijacking del flusso d'esecuzione, basta che l'instruction pointer punti all'istruzione successiva alla chiamata `x64_sys_call` nella funzione `do_syscall_x64`: nessun segnale sarà ritardato perchè il ritorno in userspace non viene saltato dall'Hijacking, perciò in quel momento (dentro il blocco di codice `entry\_SYSCALL\_64`) tutti quelli pendenti verranno eseguiti.