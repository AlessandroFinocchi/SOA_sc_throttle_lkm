#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdatomic.h>

#define NUM_THREADS 4
#define THREAD_NAME "stress"

pthread_t threads[NUM_THREADS];
atomic_bool keep_running = ATOMIC_VAR_INIT(true);

void alarm_handler() {
    // Il kernel eseguirà comunque il passaggio per il signal delivery
}

void* syscalls_routine(void *index) {
    char* name = NULL;

    // asprintf allocates memory and points thread_name_ptr to it
    // It returns the number of bytes printed, or -1 on allocation failure.    
    if ((asprintf(&name, "%s%d", THREAD_NAME, *(int *)index) == -1)) {
        // asprintf leaves the pointer undefined on failure, ensure it is NULL
        return NULL; 
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int ms_sleep = 50;
    int duration = 10 * 1000 / ms_sleep;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        pid_t pid = getpid();
        raise(SIGALRM);
        
        printf("Thread [%s] - PID [%d] - chiamata PID numero %d\n", name, pid, i);
        nanosleep(&ts, NULL);
    }

    atomic_store(&keep_running, false);
    free(name);
    return NULL;
}

void* alarm_routine() {
    char* name = NULL;

    // asprintf allocates memory and points thread_name_ptr to it
    // It returns the number of bytes printed, or -1 on allocation failure.    
    if ((asprintf(&name, "%s", "no_stress") == -1)) {
        // asprintf leaves the pointer undefined on failure, ensure it is NULL
        return NULL;
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int ms_sleep = 10;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    while (atomic_load(&keep_running)) {
        // Invia il segnale SIGALRM specificamente al Thread 1

        for(int i = 0; i < NUM_THREADS; i++){
            pthread_kill(threads[i], SIGALRM);
        }
        
        // Cadenza l'invio dei segnali
        nanosleep(&ts, NULL);
    }
    
    free(name);
    return NULL;
}

int main() {
    pthread_t alrm_thread;
    int thread_args[NUM_THREADS];
    
    // Registrazione dell'handler per SIGALRM
    // Senza questo, il primo thread che chiama raise() terminerebbe l'intero processo.
    signal(SIGALRM, alarm_handler);

    for (long t = 0; t < NUM_THREADS; t++) {
        thread_args[t] = t+1;
        if (pthread_create(&threads[t], NULL, syscalls_routine, (void *)&thread_args[t]) != 0) {
            perror("pthread_create");
            return -1;
        }
    }
    
    if (pthread_create(&alrm_thread, NULL, alarm_routine, NULL) != 0) {
        perror("Errore creazione Thread alrm_thread");
        return 1;
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
        pthread_join(alrm_thread, NULL);

    return 0;
}