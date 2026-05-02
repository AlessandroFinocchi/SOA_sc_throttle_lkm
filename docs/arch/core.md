# Sottosistema Core

## 1. Introduzione
Il sottosistema core è deputato all'ispezione dei flussi d'esecuzione delle system call intercettate dal sottosistema probes, per valutarne la compatibilità con la policy di rate-limiting configurata, e in caso di eccesso della soglia, gestisce la sospensione del thread chiamante.

## 2. Blocco dei thread
Il blocco dei thread, compatibilmente alle regole di throttling, viene gestito attraverso l'utilizzo di primitive di sincronizzazione native del kernel, nello specifico le **wait-event queues**:

- **Sospensione del thread**: Qualora un thread non riesce ad ottenere un token, secondo l'algoritmo token-bucket, viene posto in stato di sleep sulla wait-event queue come task interrompibile, in attesa di una duplice tipologia di eventi: infatti il thread potrà tornare attivo soltanto una volta che sarà in grado di ottenere un token, oppure nel caso in cui il monitor viene spento, garantendo il corretto sblocco dei thread in fase di smontaggio del modulo.

- **Risveglio del thread**: Allo scadere di ciascuna finestra temporale, dalla durata di 1 secondo, il bucket viene riempito di tutti i suoi token, e contestualmente tutti i thread vengono risvegliati in modo da poter competere per i suddetti token in modo da poter ritornare nello stato running.

## 3. Starvation di un thread
La competizione dei thread per i token espone inevitabilmente il sistema a scenari in cui è possibile che un thread subisca **starvation** a causa del **Thundering Herd Effect**. Nel caso in cui numerosi thread entrino in competizione, questi riprenderanno il controllo della CPU seguendo il loro ordine di priorità prestabilito. In tale contesto, implementare controlli aggiuntivi per evitare la starvation potrebbe innescare fenomeni di **inversione di priorità**.

Per risolvere tale problema è possibile compilare il modulo con il flag `NO_STARVATION`, decommentando nel `Makfile` radice la relativa riga: in questo modo la gestione delle wait-event queue viene affiancata ad un **Ticket Algorithm** il quale permette una gestione FIFO della coda, in modo da eliminare la starvation.

Bisogna però considerare il seguente problema: per mantenere la coerenza in coda, i thread sono inseriti nella wait-event queue come `TASK_UNINTERRUPTIBLE`; infatti, se un thread venisse colpito da un segnale come `SIGKILL` prima che arrivi il suo turno, esso abbandonerebbe la coda senza incrementare la `ticket_tail`, causando un deadlock per tutti i thread successivi.