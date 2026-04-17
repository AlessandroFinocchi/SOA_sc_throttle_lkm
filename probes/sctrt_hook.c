#include <linux/kprobes.h>

#include "sctrt_hook.h"
#include "sctrt_kprobectx_saver.h"
#include "sctrt_state.h"
#include "sctrt_core.h"

#define target_func "x64_sys_call"

static struct kprobe sc_probe;

static int pre_hook(struct kprobe *p, struct pt_regs *the_regs) {
    if(unlikely(sctrt_check_throttling_compatibility(the_regs))) {

		__this_cpu_write(*state->kprobe_ctx_offset, NULL);
		preempt_enable();// --- INIZIO SEZIONE PREEMPTABLE ---

		printk("%s: salve a tuuuuuuuuutti ragazzi\n", MODNAME);

		preempt_disable();// --- FINE SEZIONE PREEMPTABLE ---
		__this_cpu_write(*state->kprobe_ctx_offset, p);
    }
    return 0;
}

int sctrt_hook_init(void) {
	int ret;

	if(!sctrt_save_probectx())
		return -1;
	
    printk("%s: Discovery finished. Per-CPU offset: %p\n", MODNAME, *state->kprobe_ctx_offset);

	sc_probe.symbol_name = target_func;
	sc_probe.pre_handler = (kprobe_pre_handler_t)pre_hook; // Eseguita nell'entry point della funzione

	ret = register_kprobe(&sc_probe);
	if (ret < 0) {
		printk("%s: Probes module initialization failed, returned %d\n", MODNAME, ret);
		return ret;
	}
	printk("%s: Probes module correctly loaded\n", MODNAME);
	
	return 0;
}

void sctrt_hook_exit(void) {
	unregister_kprobe(&sc_probe);

	printk("%s: Probes module unloaded\n", MODNAME);

}