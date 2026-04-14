#include <linux/module.h>
#include <linux/version.h>

#include <sctrt_dev.h>
#include <sctrt_state.h>
#include <sctrt_hook.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their invocation rate.");

static int sctrt_init(void) {
	sctrt_dev_init();
	sctrt_state_init();
	sctrt_hook_init();
	
	printk("%s: Module loaded\n", MODNAME);
	return 0;
}

static void sctrt_exit(void) {
	sctrt_hook_exit();
	sctrt_state_cleanup();
	sctrt_dev_cleanup();

	printk("%s: Module unloaded\n", MODNAME);
}

module_init(sctrt_init)
module_exit(sctrt_exit)