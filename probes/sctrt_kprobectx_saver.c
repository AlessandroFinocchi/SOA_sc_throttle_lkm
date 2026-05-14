#include <linux/kprobes.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
#include <linux/uaccess.h>

#include "sctrt.h"
#include "sctrt_kprobectx_saver.h"

struct kprobe** kprobe_ctx_offset;

// Funzione bersaglio isolata. 'noinline' impedisce al compilatore di ottimizzarla.
// 'asm("")' agisce come barriera, garantendo che il blocco non venga ottimizzato.
static noinline void dummy_target(void) {
    asm(""); 
}

static int __kprobes dummy_hook(struct kprobe *p, struct pt_regs *regs) {
	/* Brute force search for the position of the kprobe context */
	struct kprobe **temp_offset = 0;
	struct kprobe *temp;
	while (copy_from_kernel_nofault(&temp, this_cpu_ptr(temp_offset), 
                                    sizeof(struct kprobe *)) != -EFAULT) {
		/* Check if *temp_ptr points at the variable representing the
		 * kprobe context  */
		if (temp == p) {
			kprobe_ctx_offset = temp_offset;
			return 0;
		}
		temp_offset++;
	}

	return 0;
}

int sctrt_save_probectx(void) {
    struct kprobe dummy_probe;
    int status;

    // 1. Inizializzazione e registrazione della dummy kprobe
    dummy_probe.addr = (kprobe_opcode_t *)dummy_target;
    dummy_probe.pre_handler = dummy_hook;
    
    if ((status = register_kprobe(&dummy_probe))) {
        printk("%s: probes - Dummy probe registration failed: %d\n", MODNAME, status);
        return status;
    }

    // 2. Innesco sincrono: forziamo l'esecuzione della dummy kprobe
    dummy_target();

    // 3. Cleanup dummy kprobe
    unregister_kprobe(&dummy_probe);

    printk("%s: probes - Probes context discovery finished\n", MODNAME);
    return 0;
}