#include <linux/module.h>
#include <linux/version.h>

#include "sctrt.h"
#include "sctrt_dev.h"
#include "sctrt_core.h"
#include "sctrt_state.h"
#include "sctrt_hook.h"
#include "sctrt_profiler.h"
#include "sctrt_kprobectx_saver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their invocation rate.");

static int sctrt_init(void) {
	sctrt_profiler_init();
	sctrt_core_init();

	if(sctrt_dev_init()) 		goto end;
	if(sctrt_state_init()) 		goto clean_dev;
	if(sctrt_save_probectx()) 	goto clean_state; // Niente da pulire
	if(sctrt_hook_init()) 		goto clean_state;
	if(sctrt_monitor_enable()) 	goto clean_hook;
	
	printk("%s: Module loaded\n", MODNAME);

#ifdef DEBUG
	sctrt_syscall_register(39);
	sctrt_euid_register(1000);
	sctrt_prog_register("stress");
	sctrt_prog_register("stress1");
	sctrt_prog_register("stress2");
	sctrt_prog_register("stress3");
	sctrt_prog_register("stress4");
	
	sctrt_print_state();
	sctrt_print_max();
	sctrt_print_syscalls();
	sctrt_print_users();
	sctrt_print_programs();
#endif

	return 0;
clean_hook:
	sctrt_hook_exit();
clean_state: 
	sctrt_state_cleanup();
clean_dev: 
	sctrt_dev_cleanup();
end: 
	return -1;
}

static void sctrt_exit(void) {
	sctrt_monitor_disable();
	sctrt_hook_exit();
	sctrt_state_cleanup();
	sctrt_dev_cleanup();
	sctrt_core_exit();

	printk("%s: Module unloaded\n", MODNAME);
}

module_init(sctrt_init)
module_exit(sctrt_exit)