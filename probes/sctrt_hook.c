#include <linux/kprobes.h>
#include <linux/ktime.h>

#include "sctrt.h"
#include "sctrt_tb.h"
#include "sctrt_hook.h"
#include "sctrt_core.h"
#include "sctrt_state.h"
#include "sctrt_profiler.h"
#include "sctrt_kprobectx_saver.h"

#define target_func "x64_sys_call"

static struct kprobe sc_probe;

static int pre_hook(struct kprobe *p, struct pt_regs *the_regs) {

	/* Se un thread critico invoca una syscall critica */
    if(unlikely(sctrt_check_throttling_compatibility(the_regs))) {

		if(!sctrt_is_monitor_active()) return 0;

		if(!take_token()) {
			int weq_ret;
			ktime_t start_time;
		
			atomic_inc(&sctrt_in_flight);

			/* Inizio campionamento telemetria */
            start_time = ktime_get();
            sctrt_profiler_thread_sleep();

			this_cpu_write(*kprobe_ctx_offset, NULL);
			preempt_enable();// --- INIZIO SEZIONE PREEMPTABLE ---
			
			/* Blocco del thread */
			weq_ret = sctrt_wait_on_weq();

			preempt_disable();// --- FINE SEZIONE PREEMPTABLE ---
			this_cpu_write(*kprobe_ctx_offset, p);

			/* Fine campionamento telemetria */
			sctrt_profiler_thread_wakeup(start_time);

			atomic_dec(&sctrt_in_flight);

#ifndef WEQ_UNINT
			/* Se il thread è stato risvegliato da un segnale */
			if(weq_ret != 0) {
				/* Hijacking: ridirezionamento del flusso d'esecuzione verso
				 * l'istruzione successiva alla syscall invocata, il cui
				 * indirizzo è, secondo l'ABI, in cima allo stack*/
				unsigned long ret_addr = *(unsigned long *)the_regs->sp;
				the_regs->ip = ret_addr;
				the_regs->sp += sizeof(long); // Allineamento stack
				the_regs->ax = -EPERM;		  // Iniezione valore di ritorno
				return 1;					  // Annulla single-stepping
			}
#endif
		}
    }
    return 0;
}

// static void post_hook(struct kprobe *p, struct pt_regs *the_regs, unsigned long flags) {
//     /* Essenziale perchè da kernel 6.13 c'è stata un'ottimizzazione del sottosistema
// 	 * delle kprobe, come scritto nella documentazione ufficiale al link
// 	 * https://www.kernel.org/doc/Documentation/trace/kprobes.rst
// 	 *
// 	 * """
// 	 * NOTE for geeks:
// 	 * The jump optimization changes the kprobe's pre_handler behavior.
// 	 * Without optimization, the pre_handler can change the kernel's execution
// 	 * path by changing regs->ip and returning 1.  However, when the probe
// 	 * is optimized, that modification is ignored.  Thus, if you want to
// 	 * tweak the kernel's execution path, you need to suppress optimization,
// 	 * using one of the following techniques:
// 	 * 
// 	 * - Specify an empty function for the kprobe's post_handler.
// 	 * 
// 	 * or
// 	 * 
// 	 * - Execute 'sysctl -w debug.kprobes-optimization=0'
// 	 * 
// 	 * .. _kprobes_blacklist:
// 	 * """
// 	 */
// }

int sctrt_hook_init(void) {
	int status;

	if((status = token_bucket_init(2))){
		goto end;
	}

	sc_probe = (struct kprobe){0}; // Reset di tutti gli elementi a 0 per reinizializzazioni
	sc_probe.symbol_name = target_func;
	sc_probe.pre_handler = (kprobe_pre_handler_t)pre_hook; // Eseguita nell'entry point della funzione
	// sc_probe.post_handler = (kprobe_post_handler_t)post_hook;

	if ((status = register_kprobe(&sc_probe))) {
		printk("%s: probes - Initialization failed, returned %d\n", MODNAME, status);
		goto clean_tb;
	}

	printk("%s: probes - Correctly loaded\n", MODNAME);
	
	return 0;

clean_tb:
	token_bucket_exit();
end:
    return status;
}

void sctrt_hook_exit(void) {
	unregister_kprobe(&sc_probe);
	token_bucket_exit();

	printk("%s: probes - Correctly unloaded\n", MODNAME);
}