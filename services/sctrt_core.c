#include <linux/cred.h>       // Fornisce la macro current
#include <asm/syscall.h>      // Fornisce syscall_get_nr()
#include <linux/wait.h>       // Fornisce wait_queue_head_t

#ifdef PRIO_FIFO
#include <linux/atomic.h>     // Fornisce le primitive atomic64_t
#endif

#include "sctrt.h"
#include "sctrt_tb.h"
#include "sctrt_core.h"
#include "sctrt_state.h"

wait_queue_head_t sctrt_weq;

#ifdef PRIO_FIFO
#define WEQ_UNINT	/* FIFO prio => WEQ non interrompibili */
#endif

#ifdef PRIO_FIFO
/* Contatori atomici per il Ticket Algorithm */
atomic64_t sctrt_ticket_head;
atomic64_t sctrt_ticket_tail;
#endif

void sctrt_core_init() {
    init_waitqueue_head(&sctrt_weq);

#ifdef PRIO_FIFO
    /* Inizializzazione della coda FIFO */
    atomic64_set(&sctrt_ticket_head, 0);
    atomic64_set(&sctrt_ticket_tail, 0);
	printk("%s: core - WEQ FIFO prio mode configured\n", MODNAME);
#else
	printk("%s: core - WEQ Scheduler prio mode configured\n", MODNAME);
#endif 

#ifdef WEQ_UNINT
	printk("%s: core - WEQ uninterruptible configured\n", MODNAME);
#else
	printk("%s: core - WEQ interruptible configured\n", MODNAME);
#endif 

}

void sctrt_core_exit() {
    /*
     * Non dovrebbe servire perchè quando il monitor si spegne, si
     * risvegliano tutti i thread, ma per correttezza viene mantenuta.
     */
    sctrt_wake_up_weq();
}

bool sctrt_check_throttling_compatibility(struct pt_regs *regs) {
    long syscall_nr;
    char *prog_name;
    kuid_t current_euid;

    syscall_nr = syscall_get_nr(current, (struct pt_regs *)regs->di);
    prog_name = current->comm;
    current_euid = current_euid();

    /* 1. syscall check */
    if(!sctrt_is_syscall_registered(syscall_nr)) return false;

    /* * 2. process name check*/
    if(!sctrt_is_prog_registered(prog_name)) return false;

    /* * 3. euid check*/
    if (!sctrt_is_euid_registered(current_euid)) return false;

    return true;
}

int sctrt_wait_on_weq() {
    /*
     * Sospensione del thread, che non è riuscito ad ottenere un token.
     * La condizione viene rivalutata al risveglio.
     * Il risveglio avviene se il monitor non è più attivo, e quindi
     * il meccanismo di throttling deve farmarsi, oppure se al
     * risveglio si riesce a prendere un token.
     */
    int ret = 0;

#ifdef PRIO_FIFO
    /* Acquisizione atomica del ticket per il thread corrente */
    u64 my_ticket = atomic64_fetch_add(1, &sctrt_ticket_head);
#endif

#ifdef WEQ_UNINT
    /*
     * Sospensione in TASK_UNINTERRUPTIBLE per prevenire la corruzione 
     * della catena dei ticket a causa della ricezione di segnali pendenti.
     */
    wait_event(
        sctrt_weq,
        !sctrt_is_monitor_active() || 
    #ifdef PRIO_FIFO
        (atomic64_read(&sctrt_ticket_tail) == my_ticket && take_token())
    #else
        take_token()
    #endif
    );
#else
    ret = wait_event_interruptible(
        sctrt_weq,
        !sctrt_is_monitor_active() || 
        take_token()
    );
#endif

#ifdef PRIO_FIFO
    /* 
     * Il thread ha superato la barriera: passa il testimone incrementando 
     * la tail e risveglia gli altri thread affinché il prossimo 
     * nella sequenza possa valutare la propria condizione. 
     */
    atomic64_inc(&sctrt_ticket_tail);
    wake_up_all(&sctrt_weq);
#endif

    return ret;
}

void sctrt_wake_up_weq() {
    /*
     * Svuota la wait queue: risveglia tutti i thread pendenti.
     */
#ifdef WEQ_UNINT
    /* Risveglio per code TASK_UNINTERRUPTIBLE */
    wake_up_all(&sctrt_weq);
#else
    /* Risveglio per code TASK_INTERRUPTIBLE */
    wake_up_interruptible_all(&sctrt_weq);
#endif
}