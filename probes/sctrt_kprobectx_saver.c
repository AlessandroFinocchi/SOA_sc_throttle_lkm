#include <linux/kprobes.h>
#include <linux/percpu.h>
#include <linux/preempt.h>

#include "sctrt.h"
#include "sctrt_kprobectx_saver.h"

DEFINE_PER_CPU(ulong, per_cpu_var);

static struct kprobe dummy_probe;
struct kprobe** kprobe_ctx_offset;

// Funzione bersaglio isolata. 'noinline' impedisce al compilatore di ottimizzarla.
// 'asm("")' agisce come barriera, garantendo che il blocco non venga ottimizzato.
static noinline void dummy_target(void) {
    asm(""); 
}

static int dummy_hook(struct kprobe *p, struct pt_regs *regs) {
    ulong* per_cpu_ptr = (ulong*)&per_cpu_var;

    while ((ulong)per_cpu_ptr > 0) {
        per_cpu_ptr -= 1;
        // Se la variabile per-CPU in questa locazione punta alla dummy kprobe,
        // abbiamo trovato l'offset del contesto.
        if ((ulong)__this_cpu_read(*per_cpu_ptr) == (ulong)p) {
            kprobe_ctx_offset = (struct kprobe**)per_cpu_ptr;
            break;
        }
    }
    return 0;
}


int sctrt_save_probectx(void) {
    int status;

    // 1. Inizializzazione e registrazione della dummy kprobe
    dummy_probe.addr = (kprobe_opcode_t *)dummy_target;
    dummy_probe.pre_handler = dummy_hook;
    
    if ((status = register_kprobe(&dummy_probe))) {
        printk("%s: Dummy probe registration failed: %d\n", MODNAME, status);
        return status;
    }

    // 2. Innesco sincrono: forziamo l'esecuzione della dummy kprobe
    dummy_target();

    // 3. Cleanup dummy kprobe
    unregister_kprobe(&dummy_probe);

    if (*kprobe_ctx_offset == 0) {
        printk("%s: Critical error. Impossible to compute per-CPU offset.\n", MODNAME);
        status = -EINVAL;
        goto end;
    }

    printk("%s: Discovery finished. Per-CPU offset: %p\n", MODNAME, *kprobe_ctx_offset);
    return 0;


end:
    return status;
}