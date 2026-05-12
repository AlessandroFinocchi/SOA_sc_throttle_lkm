#include <linux/kprobes.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
// #include <linux/maccess.h> // Necessario per copy_from_kernel_nofault
#include <linux/uaccess.h>

#include "sctrt.h"
#include "sctrt_kprobectx_saver.h"

struct kprobe** kprobe_ctx_offset;

// Funzione bersaglio isolata. 'noinline' impedisce al compilatore di ottimizzarla.
// 'asm("")' agisce come barriera, garantendo che il blocco non venga ottimizzato.
static noinline void dummy_target(void *arg) {
    barrier();
    return;
}

static int __kprobes dummy_hook(struct kprobe *p, struct pt_regs *regs) {

    printk("%s: pre_handler_search running on CPU %d\n", MODNAME,
		 smp_processor_id());

	/* Brute force search for the position of the kprobe context */
	struct kprobe **temp_offset = 0;
	struct kprobe *temp;
    printk("AAAAAAAAAAAAAAA");
	while (copy_from_kernel_nofault(&temp, this_cpu_ptr(temp_offset),
					sizeof(struct kprobe *)) != -EFAULT) {
		/* Check if *temp_ptr points at the variable representing the
		 * kprobe context  */
		if (temp == p) {
            printk("AAAAAAAAAAAAAAA1 %p", temp_offset);
			kprobe_ctx_offset = temp_offset;
            printk("AAAAAAAAAAAAAAA1 %p", temp_offset);
			return 0;
		}
		temp_offset++;
	}

    printk("%s: pre_handler_search finished on CPU %d\n", MODNAME,
		 smp_processor_id());

	return 0;
}

// static int dummy_hook2(struct kprobe *p, struct pt_regs *regs) {
//     ulong __percpu* dyn_per_cpu_var = alloc_percpu(ulong);
//     ulong* per_cpu_ptr = (ulong*)dyn_per_cpu_var;

//     while ((ulong)per_cpu_ptr > 0) {
//         per_cpu_ptr -= 1;
//         // Se la variabile per-CPU in questa locazione punta alla dummy kprobe,
//         // abbiamo trovato l'offset del contesto.
//         if ((ulong)__this_cpu_read(*per_cpu_ptr) == (ulong)p) {
//             kprobe_ctx_offset = (struct kprobe**)per_cpu_ptr;
//             break;
//         }
//     }
    
//     free_percpu(dyn_per_cpu_var);
//     return 0;
// }


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

    printk("DIONCAENER1\n");
    // 2. Innesco sincrono: forziamo l'esecuzione della dummy kprobe
    dummy_target(NULL);

    printk("DIONCAENER2\n");

    // 3. Cleanup dummy kprobe
    unregister_kprobe(&dummy_probe);

    printk("%s: probes - Probes context discovery finished. Per-CPU offset: %p\n", MODNAME, this_cpu_read(*kprobe_ctx_offset));
    return 0;
}