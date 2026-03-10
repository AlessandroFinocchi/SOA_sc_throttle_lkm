# System call Throttling LKM

## Specifica

In questa repository è present un Linux Kernel Module per implementare un meccanismo di system call throttling. L'LKM deve offrire un device driver per supportare la (de)registrazione di:
- **Nomi di programmi** (eseguibili)
- **User ID** (effective)
- **Syscall numbers** (in accordo alla specifica x86_64)

I syscall numbers sono usati per indicare system call critiche sotto il punto di vista di scalabilità, performance o sicurezza.<br>
Ogni volta che una syscall registrata è invocata da un programma o un utente registrato nel device driver, la reale esecuzione è controllata da un **monitor** offerto dall'LKM.
<br>
L'LKM può essere configurato per indicare il massimo numero *MAX*, per le syscall registrate, che possono essere invocate in una **finestra temporale** di 1 secondo da un programma/utente registrato. Se il numero di invocazioni nella finestra supera *MAX*, il thread coinvolto deve essere bloccato, prevenendo l'effettiva invocazione.
<br>
Le system call possono essere di qualsiasi **natura**, bloccante o meno.
<br>
Il device driver deve permettere ad ogni istante di tempo di verificare i programmi, utenti, e syscall numbers registrati. Inoltre, l'aggiornamento di queste informazioni può avvenire soltanto da parte di un thread che esegue con effective UserID pari a 0 (root).
<br>
Il device driver deve supportare la possibilità di **accendere/spegnere** il monitor delle syscall. Quando il monitor è spento, non c'è limite alla frequenza di invocazione delle syscall registrate.
<br>
In più, sulla base del valore di *MAX* selezionato, il device driver deve anche mettere a disposizione dati riguardanti:
- Il **massimo delay** per l'esecuzione effettiva di una syscall invocata, con il corrispondente programma e user ID
- Il **numero massimo** e **medio** di thread che sono stati bloccati

Della repository fanno parte anche il codice user-level per configurare l'LKM e per testare il corretto funzionamento del software sviluppato.