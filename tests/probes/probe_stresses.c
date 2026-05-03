#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
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

void* stress_routine(void *index) {
    char* name = NULL;

    // asprintf allocates memory and points thread_name_ptr to it
    // It returns the number of bytes printed, or -1 on allocation failure.    
    if ((asprintf(&name, "%s%d", THREAD_NAME, *(int *)index) == -1)) {
        // asprintf leaves the pointer undefined on failure, ensure it is NULL
        return NULL; 
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int ms_sleep = 500;
    int duration = 10 * 1000 / ms_sleep;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        pid_t pid = getpid();
        raise(SIGALRM);
        
        printf("Thread [%s] - PID [%d] - chiamata PID numero %d\n", THREAD_NAME, pid, i);
        nanosleep(&ts, NULL);
    }

    free(name);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    
    // Registrazione dell'handler per SIGALRM
    // Senza questo, il primo thread che chiama raise() terminerebbe l'intero processo.
    signal(SIGALRM, alarm_handler);

    for (long t = 0; t < NUM_THREADS; t++) {
        thread_args[t] = t+1;
        if (pthread_create(&threads[t], NULL, stress_routine, (void *)&thread_args[t]) != 0) {
            perror("pthread_create");
            return -1;
        }
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    return 0;
}