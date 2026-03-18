#include <linux/module.h>
#include <linux/version.h>

#include <sctrt_dev.h>
#include <sctrt_state.h>

#define MODNAME "sctrt"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their system call\
                    invocation rate.");

static int sctrt_init(void) {
	// if (LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)){
	//  	printk("%s: unsupported kernel version", MODNAME);
	// 	return -1; 	
	// };

	// sctrt_dev_init();
	sctrt_state_init(256);
	
	printk("%s: syscall throttler module loaded\n", MODNAME);

	return 0;
}

static void sctrt_exit(void) {
	// sctrt_dev_cleanup();
	sctrt_state_cleanup();
	printk("%s: syscall throttler module unloaded\n", MODNAME);
}

module_init(sctrt_init)
module_exit(sctrt_exit)