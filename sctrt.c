#include <linux/module.h>
#include <linux/version.h>

#include <sctrt_dev.h>
#include <sctrt_state.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessandro Finocchi <alessandro.22082001@gmail.it>");
MODULE_DESCRIPTION("This module intercepts a set of syscall from a set\
                    of programs and users to limit their invocation rate.");

void sctrt_print_conf(void);

static int sctrt_init(void) {
	sctrt_dev_init();
	sctrt_state_init(256);	// TODO: SOSTITUIRE CON IL NUMERO DI SYSCALL MASSIMO!

	sctrt_print_conf();
	sctrt_monitor_enable();
	sctrt_max_set(100);
	sctrt_syscall_register(42);
	sctrt_syscall_register(43);
	sctrt_prog_register("Io");
	sctrt_prog_register("me");
	sctrt_euid_register(42);
	sctrt_euid_register(43);
	sctrt_print_conf();
	
	printk("%s: Module loaded\n", MODNAME);
	return 0;
}

static void sctrt_exit(void) {
	sctrt_state_cleanup();
	sctrt_dev_cleanup();

	printk("%s: Module unloaded\n", MODNAME);
}

void sctrt_print_conf() {
	printk("%s: ---------- Config ----------\n", MODNAME);
	sctrt_print_state();
	sctrt_print_max();
	sctrt_print_syscalls();
	sctrt_print_users();
	sctrt_print_programs();
}

module_init(sctrt_init)
module_exit(sctrt_exit)