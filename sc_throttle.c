#include <linux/module.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their system call\
                    invocation rate.");

#define MODNAME "SC-THROTTLER"

static int module_init(void) {
	if (LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)){
	 	printk("%s: unsupported kernel version", MODNAME);
		return -1; 	
	};
	
	return 0;
}

static void hook_exit(void) {
	printk("%s: syscall throttler module unloaded\n", MODNAME);
}

module_init(hook_init)
module_exit(hook_exit)
