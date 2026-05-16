# Sottosistema Core

## 1. Introduzione
Il sottosistema core è deputato all'ispezione dei flussi d'esecuzione delle system call intercettate dal sottosistema probes, per valutarne la compatibilità con la policy di rate-limiting configurata, e in caso di eccesso della soglia, gestisce la sospensione del thread chiamante.

## 2. Blocco dei thread
Il blocco dei thread, compatibilmente alle regole di throttling, viene gestito attraverso l'utilizzo di primitive di sincronizzazione native del kernel, nello specifico le **wait-event queues**:

- **Sospensione del thread**: Qualora un thread non riesce ad ottenere un token, secondo l'algoritmo token-bucket, viene posto in stato di sleep sulla wait-event queue come task interrompibile, in attesa di una duplice tipologia di eventi: infatti il thread potrà tornare attivo soltanto una volta che sarà in grado di ottenere un token, oppure nel caso in cui il monitor viene spento, garantendo il corretto sblocco dei thread in fase di smontaggio del modulo.

- **Risveglio del thread**: Allo scadere di ciascuna finestra temporale, dalla durata di 1 secondo, il bucket viene riempito di tutti i suoi token, e contestualmente tutti i thread vengono risvegliati in modo da poter competere per i suddetti token in modo da poter ritornare nello stato running.


## 3. Gestione dei segnali
Per un thread la gestione dei segnali è un aspetto che può risultare critico, e dunque è importante che il modulo permetta ad un thread di dormire in modalità `TASK_INTERRUPTIBLE` sulla wait-event queue.

Al tempo stesso, nel contesto di questo LKM, non considerare questo fattore può portare ad una vulnerabilità del modulo stesso: infatti, senza alcuna gestione dei segnali, un thread le cui system call dovrebbero essere intercettate potrebbe sopraffare il meccanismo di throttling tramite un bombardamento di segnali innocui, che lo risveglierebbero costantamente permettendogli di fatto di bypassare la barriera di rate-limiting introdotta dal modulo.

A tale scopo, il modulo è stato reso configurabile a tempo di compilazione:
- Se la macro `WEQ_UNINT` **non è definita**, le wait-event queue saranno non interrompibli.
- Se la macro `WEQ_UNINT` **è definita**, le wait event queue saranno interrompibili.

A coronare la gestione dei segnali c'è anche il meccanismo di **Hijacking del fluss d'esecuzione** aggiunto nel [Sottosistema Kprobes](probes.md) (vedere la relativa sottosezione), in modo da rendere robusto il throttling anche in caso di wait-event queue interrompibili.


## 4. Starvation di un thread
La competizione dei thread per i token espone inevitabilmente il sistema a scenari in cui è possibile che un thread subisca **starvation** a causa del **Thundering Herd Effect**. Nel caso in cui numerosi thread entrino in competizione, questi riprenderanno il controllo della CPU seguendo il loro ordine di priorità prestabilito. In tale contesto, implementare controlli aggiuntivi per evitare la starvation potrebbe innescare fenomeni di **inversione di priorità**.

Per indirizzare tale problema, il modulo è stato reso configurabile a tempo di compilazione:
- Se la macro `PRIO_FIFO` **non è definita**, la strategia di risveglio dalla wait-event queue sarà quella decisa dallo scheduler, in modo da prevenire fenomeni di **inversione di priorità**. Dunque, in questo contesto persisterà il problema di **starvation**.
- Se la macro `PRIO_FIFO` **è definita**, il modulo affiancherà alla gestione delle waite-event queue un **Ticket Algorithm** il quale permette una gestione FIFO della coda, in modo da eliminare la starvation. Dunque, in questo contesto persisterà il problema di **inversione di priorità**. Inoltre, c'è da aggiungere che per mantenere la coerenza in coda, i thread sono inseriti nella wait-event queue come `TASK_UNINTERRUPTIBLE`; infatti, se un thread venisse colpito da un segnale come `SIGKILL` prima che arrivi il suo turno, esso abbandonerebbe la coda senza incrementare la `ticket_tail`, causando un deadlock per tutti i thread successivi.
