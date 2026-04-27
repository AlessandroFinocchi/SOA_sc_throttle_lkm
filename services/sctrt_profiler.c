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

void sctrt_profiler_init(void) {
    sctrt_profiler_reset();
    printk("%s: profiler - Initialized\n", MODNAME);
}

void sctrt_profiler_reset(void) {
    atomic_set(&current_blocked_threads, 0);
    atomic_set(&peak_blocked_threads, 0);
    atomic64_set(&sum_blocked_threads, 0);
    atomic_set(&total_samples, 0);
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

void sctrt_profiler_thread_wakeup(void) {
    atomic_dec(&current_blocked_threads);
}

void sctrt_profiler_sample_window(void) {
    int current_blocked = atomic_read(&current_blocked_threads);
    atomic64_add(current_blocked, &sum_blocked_threads);
    atomic_inc(&total_samples);
}

void sctrt_profiler_get(struct sc_throttle_param *param) {    
    param->data.profiler.peak_blocked_threads = atomic_read(&peak_blocked_threads);
    param->data.profiler.sum_blocked_threads = atomic64_read(&sum_blocked_threads);
    param->data.profiler.total_samples = atomic_read(&total_samples);
}