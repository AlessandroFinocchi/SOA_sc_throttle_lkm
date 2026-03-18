#include <linux/kprobes.h>

#include "sctrt_hook.h"
#include "sctrt_state.h"

#define target_func "x64_sys_call"

static struct kretprobe retprobe;  
static struct kretprobe *the_retprobe = &retprobe;  

static int pre_hook(struct kprobe *ri, struct pt_regs *the_regs) { 
	return 0;
}

static int sctrt_hook_init(void) {
	int ret;

	retprobe.kp.symbol_name = target_func;
	retprobe.entry_handler = (kretprobe_handler_t)pre_hook; // Eseguita nell'entry point della funzione
	retprobe.maxactive = -1;                                // Numero di kretprobes gestibile di default dal kernel

	ret = register_kretprobe(&retprobe);
	if (ret < 0) {
		printk("%s: Probes module initialization failed, returned %d\n", MODNAME, ret);
		return ret;
	}
	printk("%s: Probes module correctly loaded\n", MODNAME);
	
	return 0;
}

static void sctrt_hook_exit(void) {
	unregister_kretprobe(&retprobe);

	printk("%s: Probes module unloaded\n", MODNAME);

}

static struct kretprobe retprobe;  
static struct kretprobe *the_retprobe = &retprobe;