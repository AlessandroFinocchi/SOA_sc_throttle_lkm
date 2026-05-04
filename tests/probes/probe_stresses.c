/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#ifdef ATTACK
#include <stdatomic.h>
#endif

#define NUM_THREADS 4
#define THREAD_NAME "stress"

pthread_t threads[NUM_THREADS];

#ifdef ATTACK
atomic_bool keep_running = ATOMIC_VAR_INIT(true);

void alarm_handler() {
    fflush(stdout);
}
#endif

void* syscalls_routine(void *index) {
    char* name = NULL;

    /* asprintf alloca anche la memoria, -1 se l'allocazione fallisce*/    
    if ((asprintf(&name, "%s%d", THREAD_NAME, *(int *)index) == -1)) {
        return NULL; 
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int ms_sleep = 50;
    int duration = 30 * 1000 / ms_sleep;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        pid_t pid = getpid();
        
        printf("Thread [%s] - PID [%d] - chiamata PID numero %d\n", name, pid, i);
        fflush(stdout);
        nanosleep(&ts, NULL);
    }

    // Al termine dell'iterazione, segnala all'alarm thread di arrestarsi
#ifdef ATTACK
    atomic_store(&keep_running, false);
#endif
    free(name);
    return NULL;
}

#ifdef ATTACK
void* alarm_routine() {
    char* name = NULL;

    /* asprintf alloca anche la memoria, -1 se l'allocazione fallisce*/  
    if ((asprintf(&name, "%s", "no_stress") == -1)) {
        return NULL;
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int ms_sleep = 10;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    while (atomic_load(&keep_running)) {
        // Invia il segnale SIGALRM agli altri thread
        for(int i = 0; i < NUM_THREADS; i++){
            pthread_kill(threads[i], SIGALRM);
        }
        
        // Cadenza l'invio dei segnali
        nanosleep(&ts, NULL);
    }
    
    free(name);
    return NULL;
}
#endif

int main() {
    int thread_args[NUM_THREADS];
    
#ifndef ATTACK
    printf("Attacco stresses disabilitato\n");
#else
    printf("Attacco stresses abilitato\n");
    pthread_t alrm_thread;
    // Gli handler (signal disposition) sono condivisi a livello di processo.
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Nessun flag SA_RESTART: vogliamo che il segnale interrompa eventuali syscall
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Errore nella registrazione di sigaction");
        return 1;
    }
#endif

    for (long t = 0; t < NUM_THREADS; t++) {
        thread_args[t] = t+1;
        if (pthread_create(&threads[t], NULL, syscalls_routine, (void *)&thread_args[t]) != 0) {
            perror("pthread_create");
            return -1;
        }
    }
    
#ifdef ATTACK
    if (pthread_create(&alrm_thread, NULL, alarm_routine, NULL) != 0) {
        perror("Errore creazione Thread alrm_thread");
        return 1;
    }
#endif

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

#ifdef ATTACK
    pthread_join(alrm_thread, NULL);
#endif

    printf("Esecuzione terminata regolarmente.\n");
    return 0;
}