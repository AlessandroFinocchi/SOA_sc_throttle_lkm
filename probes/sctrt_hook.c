#include <linux/kprobes.h>

#include "sctrt_hook.h"
#include "sctrt_kprobectx_saver.h"
#include "sctrt_state.h"
#include "sctrt_core.h"
#include "sctrt_tb.h"


#define target_func "x64_sys_call"

static struct kprobe sc_probe;

void thread_migration_fn(void);

static int pre_hook(struct kprobe *p, struct pt_regs *the_regs) {
    if(unlikely(sctrt_check_throttling_compatibility(the_regs))) {
		// if(!take_token()) {
			pr_info("Entrata: Kprobe '%s' in esecuzione sulla CPU %u\n", p->symbol_name, smp_processor_id());
			__this_cpu_write(*kprobe_ctx_offset, NULL);
			preempt_enable();// --- INIZIO SEZIONE PREEMPTABLE ---

			printk("%s: salve a tuuuuuuuuutti ragazzi\n", MODNAME);
			thread_migration_fn();

			preempt_disable();// --- FINE SEZIONE PREEMPTABLE ---
			__this_cpu_write(*kprobe_ctx_offset, p);
			pr_info("Uscita: Kprobe '%s' in esecuzione sulla CPU %u\n", p->symbol_name, smp_processor_id());
		// }
    }
    return 0;
}

int sctrt_hook_init(void) {
	int ret;

	// if(!token_bucket_init())
	// 	return -1;

    if(!(kprobe_ctx_offset = kzalloc(sizeof(struct kprobe*), GFP_KERNEL))) {
        return -ENOMEM;
	}

	if(!sctrt_save_probectx()) {
		return -1;
	}
	
	sc_probe.symbol_name = target_func;
	sc_probe.pre_handler = (kprobe_pre_handler_t)pre_hook; // Eseguita nell'entry point della funzione

	ret = register_kprobe(&sc_probe);
	if (ret < 0) {
		printk("%s: Probes module initialization failed, returned %d\n", MODNAME, ret);
		return ret;
	}
	printk("%s: Probes module correctly loaded\n", MODNAME);
	
	return 0;
}

void sctrt_hook_exit(void) {
	unregister_kprobe(&sc_probe);
    kfree(kprobe_ctx_offset);
	// token_bucket_exit();

	printk("%s: Probes module unloaded\n", MODNAME);

}


//-------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/sched.h>
void thread_migration_fn(){
    int old_cpu, new_cpu, target_cpu;
    cpumask_t mask;

    old_cpu = get_cpu();
    put_cpu();
    
    pr_info("[KThread] Esecuzione avviata. old_cpu = %d\n", old_cpu);

    target_cpu = -1;
    for_each_online_cpu(target_cpu) {
        if (target_cpu != old_cpu && target_cpu != old_cpu) {
            break; // Trovata una CPU idonea
        }
    }

    if (target_cpu >= nr_cpu_ids || target_cpu == -1) {
        pr_err("[KThread] Nessuna CPU valida disponibile per la migrazione.\n");
        return;
    }

    pr_info("[KThread] Preparazione della migrazione verso la CPU %d...\n", target_cpu);

    /* * 3. Configurazione della maschera di affinità e rischedulazione.
     * Limitando l'affinità alla sola CPU target, forziamo lo scheduler 
     * a migrare questo thread.
     */
    cpumask_clear(&mask);
    cpumask_set_cpu(target_cpu, &mask);

    /* * set_cpus_allowed_ptr è la funzione core per cambiare l'affinità.
     * Se il task si trova su una CPU che non fa più parte della maschera,
     * la funzione si blocca, sveglia il migration thread (migration/N) 
     * della CPU corrente e attende che il task venga spostato fisicamente.
     */
    if (set_cpus_allowed_ptr(current, &mask) != 0) {
        pr_err("[KThread] Errore durante set_cpus_allowed_ptr.\n");
        return;
    }

    /* * 4. Conferma della migrazione.
     * A questo punto, l'istruzione successiva viene eseguita sulla nuova CPU.
     */
    new_cpu = get_cpu();
    put_cpu();

    pr_info("[KThread] Migrazione completata con successo. new_cpu = %d\n", new_cpu);
}