#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#define NUM_THREADS 4
#define THREAD_NAME "stress"

void alarm_handler() {
    // Il kernel eseguirà comunque il passaggio per il signal delivery
}

void* stress_routine() {
    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), THREAD_NAME);

    int ms_sleep = 500;
    int duration = 10 * 1000 / ms_sleep;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        pid_t pid = getpid();
        raise(SIGALRM);
        
        printf("Thread [%s] - PID: %d (SIGALRM inviato)\n", THREAD_NAME, pid);
        nanosleep(&ts, NULL);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    
    // Registrazione dell'handler per SIGALRM
    // Senza questo, il primo thread che chiama raise() terminerebbe l'intero processo.
    signal(SIGALRM, alarm_handler);

    for (long t = 0; t < NUM_THREADS; t++) {
        if (pthread_create(&threads[t], NULL, stress_routine, NULL) != 0) {
            perror("pthread_create");
            return -1;
        }
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    return 0;
}