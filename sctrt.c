#include <linux/module.h>
#include <linux/version.h>

#include "sctrt.h"
#include "sctrt_dev.h"
#include "sctrt_core.h"
#include "sctrt_state.h"
#include "sctrt_kprobectx_saver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their invocation rate.");

static int sctrt_init(void) {
	sctrt_core_init();

	if(sctrt_dev_init()) 		goto end;
	if(sctrt_state_init()) 		goto clean_dev;
	if(sctrt_save_probectx()) 	goto clean_state;	// Niente da pulire
	if(sctrt_monitor_enable()) 	goto clean_state;
	
	printk("%s: Module loaded\n", MODNAME);
	return 0;

clean_state: 
	sctrt_state_cleanup();
clean_dev: 
	sctrt_dev_cleanup();
end: 
	return -1;
}

static void sctrt_exit(void) {
	sctrt_monitor_disable();
	sctrt_state_cleanup();
	sctrt_dev_cleanup();
	sctrt_core_exit();

	printk("%s: Module unloaded\n", MODNAME);
}

module_init(sctrt_init)
module_exit(sctrt_exit)