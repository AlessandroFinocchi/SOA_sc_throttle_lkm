#define _GNU_SOURCE // Necessario per sched_setaffinity
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/syscall.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define ITERATIONS (1000 * 1000 * 10)

int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Funzione per isolare il processo e mimare la non-preemption
void setup_isolated_environment() {
    // 1. Lock della memoria per prevenire Page Faults
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        fprintf(stderr, "Attenzione: mlockall fallito (%s). Esegui come root?\n", strerror(errno));
    }

    // 2. Impostazione dell'affinità per vincolare il thread al Core 0
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &set) != 0) {
        fprintf(stderr, "Attenzione: sched_setaffinity fallito (%s)\n", strerror(errno));
    }

    // 3. Impostazione della policy di scheduling Real-Time (SCHED_FIFO)
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        fprintf(stderr, "Attenzione: sched_setscheduler fallito (%s). Esegui con sudo per SCHED_FIFO.\n", strerror(errno));
    }
}

int main() {
    // Configurazione dell'ambiente Real-Time
    setup_isolated_environment();

    struct timespec start, end;
    double *times = malloc(ITERATIONS * sizeof(double));
    
    if (!times) {
        perror("Errore nell'allocazione della memoria");
        return EXIT_FAILURE;
    }

    // Fase di warm-up
    for(int i = 0; i < 10000; i++) {
        syscall(SYS_read);
    }

    // Loop di misurazione
    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        syscall(SYS_read); 
        clock_gettime(CLOCK_MONOTONIC, &end);

        times[i] = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    }

    // Calcolo Statistiche (O(N))
    double sum = 0.0;
    double min = times[0];
    double max = times[0];

    for (int i = 0; i < ITERATIONS; i++) {
        sum += times[i];
        if (times[i] < min) min = times[i];
        if (times[i] > max) max = times[i];
    }

    double mean = sum / ITERATIONS;

    // Varianza e Deviazione Standard (O(N))
    double sum_sq_diff = 0.0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum_sq_diff += (times[i] - mean) * (times[i] - mean);
    }
    double variance = sum_sq_diff / (ITERATIONS - 1);
    double std_dev = sqrt(variance);

    // Intervallo di Confidenza al 95%
    double standard_error = std_dev / sqrt(ITERATIONS);
    double margin_of_error = 1.96 * standard_error;

    // Mediana: richiede ordinamento (O(N log N))
    qsort(times, ITERATIONS, sizeof(double), compare_doubles);
    double median = (ITERATIONS % 2 == 0) 
        ? (times[ITERATIONS / 2 - 1] + times[ITERATIONS / 2]) / 2.0 
        : times[ITERATIONS / 2];

    // Output
    printf("Statistiche su %d iterazioni con ambiente isolato (ns):\n", ITERATIONS);
    printf("---------------------------------------------------\n");
    printf("Minimo   : %.2f ns\n", min);
    printf("Massimo  : %.2f ns\n", max);
    printf("Media    : %.2f ns\n", mean);
    printf("Mediana  : %.2f ns\n", median);
    printf("Dev. Std.: %.2f ns\n", std_dev);
    printf("CI (95%%) : [%.2f ns, %.2f ns]\n", mean - margin_of_error, mean + margin_of_error);

    free(times);
    return EXIT_SUCCESS;
}