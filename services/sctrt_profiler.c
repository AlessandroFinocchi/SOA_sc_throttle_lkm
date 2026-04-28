#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/seqlock.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/string.h>

#include "sctrt.h"
#include "sctrt_profiler.h"

/* Variabili per il calcolo del massimo campionario */
static atomic_t current_blocked_threads;
static atomic_t peak_blocked_threads;

/* Variabili per il calcolo della media campionaria */
static atomic64_t sum_blocked_threads;
static atomic_t total_samples;

/* * SeqLock per la consistenza in scrittura/lettura dei dati di picco.
 * Garantisce che un lettore (ioctl) non ottenga mai una struttura 
 * ibrida mentre un thread (kprobe handler) sta aggiornando i record.
 * Lettori lockless, writer senza starvation.
 */
static seqlock_t peak_delay_seqlock;
static unsigned long peak_delay_ns;
static char peak_prog_name[MAX_PROG_NAME_LEN];
static kuid_t peak_uid;

void sctrt_profiler_init(void) {
    seqlock_init(&peak_delay_seqlock);
    sctrt_profiler_reset();
    printk("%s: profiler - Initialized\n", MODNAME);
}

void sctrt_profiler_reset(void) {
    atomic_set(&current_blocked_threads, 0);
    atomic_set(&peak_blocked_threads, 0);
    atomic64_set(&sum_blocked_threads, 0);
    atomic_set(&total_samples, 0);

    write_seqlock(&peak_delay_seqlock);
    peak_delay_ns = 0;
    memset(peak_prog_name, 0, MAX_PROG_NAME_LEN);
    peak_uid = KUIDT_INIT(0);
    write_sequnlock(&peak_delay_seqlock);
}

void sctrt_profiler_thread_sleep(void) {
    int peak;
    int current_blocked = atomic_inc_return(&current_blocked_threads);
    
    /* Algoritmo lock-free per la massimizzazione: Compare-And-Swap loop */
    peak = atomic_read(&peak_blocked_threads);
    while (current_blocked > peak) {
        if (likely(atomic_try_cmpxchg(&peak_blocked_threads, &peak, current_blocked))) {
            break;
        }
        /* Se cmpxchg fallisce, 'peak' viene aggiornato al valore corrente, e il ciclo ripete */
    }
}

void sctrt_profiler_thread_wakeup(ktime_t start_time) {
    ktime_t end_time = ktime_get();
    s64 delay_ns = ktime_to_ns(ktime_sub(end_time, start_time));

    atomic_dec(&current_blocked_threads);

    /* * Lettura ottimistica fuori dal lock per evitare contention. 
     * Se il limite è superato, si entra nella sezione critica e si verifica nuovamente.
     */
    if (unlikely(delay_ns > peak_delay_ns)) {
        write_seqlock(&peak_delay_seqlock);
        if (delay_ns > peak_delay_ns) {
            peak_delay_ns = (unsigned long)delay_ns;
            strscpy(peak_prog_name, current->comm, MAX_PROG_NAME_LEN);
            peak_uid = current_euid();
        }
        write_sequnlock(&peak_delay_seqlock);
    }
}

void sctrt_profiler_sample_window(void) {
    int current_blocked = atomic_read(&current_blocked_threads);
    atomic64_add(current_blocked, &sum_blocked_threads);
    atomic_inc(&total_samples);
}

void sctrt_profiler_get(struct sc_throttle_param *param) {
    unsigned int seq;
    unsigned long delay;
    char prog_name[MAX_PROG_NAME_LEN];
    kuid_t euid;

    /* Dati per ottenere massimo e media dei thread bloccati*/
    param->data.profiler.peak_blocked_threads = atomic_read(&peak_blocked_threads);
    param->data.profiler.sum_blocked_threads = atomic64_read(&sum_blocked_threads);
    param->data.profiler.total_samples = atomic_read(&total_samples);

    /* Lettura wait-free protetta da SeqLock */
    do {
        seq = read_seqbegin(&peak_delay_seqlock);
        delay = peak_delay_ns;
        strscpy(prog_name, peak_prog_name, MAX_PROG_NAME_LEN);
        euid = peak_uid;
    } while (read_seqretry(&peak_delay_seqlock, seq));

    param->data.profiler.peak_delay_ns = delay;
    strscpy(param->data.profiler.peak_prog_name, prog_name, MAX_PROG_NAME_LEN);
    param->data.profiler.peak_uid = (uint) euid.val;
}