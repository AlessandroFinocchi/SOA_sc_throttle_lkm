# System call Throttling LKM

## Specifica

In questa repository è present un Linux Kernel Module per implementare un meccanismo di system call throttling. L'LKM deve offrire un device driver per supportare la (de)registrazione di:
- **Nomi di programmi** (eseguibili)
- **User ID** (effective)
- **Syscall numbers** (in accordo alla specifica x86_64)

I syscall numbers sono usati per indicare system call critiche sotto il punto di vista di scalabilità, performance o sicurezza.<br>
Ogni volta che una syscall registrata è invocata da un programma o un utente registrato nel device driver, la reale esecuzione è controllata da un **monitor** offerto dall'LKM.

L'LKM può essere configurato per per indicare il massimo numero *MAX*, per le syscall registrate, che possono essere invocate in una finestra temporale di 1 secondo da un programma/utente registrato. Se il numero di invocazioni nella finestra supera *MAX*, il thread coinvolto deve essere bloccato, prevenendo l'effettiva invocazione.

