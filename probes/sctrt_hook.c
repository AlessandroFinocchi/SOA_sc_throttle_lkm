#include <linux/kprobes.h>

#include "sctrt.h"
#include "sctrt_hook.h"
#include "sctrt_kprobectx_saver.h"
#include "sctrt_state.h"
#include "sctrt_core.h"
#include "sctrt_tb.h"


#define target_func "x64_sys_call"

static struct kprobe sc_probe;

void thread_migration_fn(void);

static int pre_hook(struct kprobe *p, struct pt_regs *the_regs) {
    if(unlikely(sctrt_check_throttling_compatibility(the_regs))) {
		// if(!take_token()) {
			pr_info("Entrata: Kprobe '%s' in esecuzione sulla CPU %u\n", p->symbol_name, smp_processor_id());
			__this_cpu_write(*kprobe_ctx_offset, NULL);
			preempt_enable();// --- INIZIO SEZIONE PREEMPTABLE ---

			printk("%s: salve a tuuuuuuuuutti ragazzi\n", MODNAME);

			preempt_disable();// --- FINE SEZIONE PREEMPTABLE ---
			__this_cpu_write(*kprobe_ctx_offset, p);
			pr_info("Uscita: Kprobe '%s' in esecuzione sulla CPU %u\n", p->symbol_name, smp_processor_id());
		// }
    }
    return 0;
}

int sctrt_hook_init(void) {
	int status;

	// if(!token_bucket_init())
	// 	return -1;

	if((status = sctrt_save_probectx())) {
		goto end;
	}
	
	sc_probe.symbol_name = target_func;
	sc_probe.pre_handler = (kprobe_pre_handler_t)pre_hook; // Eseguita nell'entry point della funzione

	if ((status = register_kprobe(&sc_probe))) {
		printk("%s: Probes module initialization failed, returned %d\n", MODNAME, status);
		goto end;
	}

	printk("%s: Probes module correctly loaded\n", MODNAME);
	
	return 0;


end:
    return status;
}

void sctrt_hook_exit(void) {
	unregister_kprobe(&sc_probe);

	printk("%s: Probes module unloaded\n", MODNAME);
}