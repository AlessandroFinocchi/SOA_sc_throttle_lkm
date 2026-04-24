#include <linux/kprobes.h>

#include "sctrt.h"
#include "sctrt_hook.h"
#include "sctrt_kprobectx_saver.h"
#include "sctrt_core.h"
#include "sctrt_tb.h"

#define target_func "x64_sys_call"

static struct kprobe sc_probe;

void thread_migration_fn(void);

static int pre_hook(struct kprobe *p, struct pt_regs *the_regs) {
    if(unlikely(sctrt_check_throttling_compatibility(the_regs))) {
		if(!take_token()) {
			__this_cpu_write(*kprobe_ctx_offset, NULL);
			preempt_enable();// --- INIZIO SEZIONE PREEMPTABLE ---

			sctrt_wait_on_weq();

			preempt_disable();// --- FINE SEZIONE PREEMPTABLE ---
			__this_cpu_write(*kprobe_ctx_offset, p);
		}
		else
			printk("%s: salve a tuuuuuuuuutti ragazzi\n", MODNAME);
    }
    return 0;
}

int sctrt_hook_init(void) {
	int status;
	if((status = token_bucket_init(10))){
		goto end;
	}
	
	sc_probe = (struct kprobe){0}; // Reset di tutti gli elementi a 0 per reinizializzazioni
	sc_probe.symbol_name = target_func;
	sc_probe.pre_handler = (kprobe_pre_handler_t)pre_hook; // Eseguita nell'entry point della funzione

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