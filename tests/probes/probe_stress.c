/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#ifdef ATTACK
#include <stdatomic.h>
#endif

#define THREAD_NAME "stress"

pthread_t syscalls_thread;
#ifdef ATTACK
atomic_bool keep_running = ATOMIC_VAR_INIT(true);

void alarm_handler() {
    fflush(stdout);
}
#endif

void* syscalls_routine() {
    char* name = NULL;

    /* asprintf alloca anche la memoria, -1 se l'allocazione fallisce*/     
    if ((asprintf(&name, "%s", THREAD_NAME) == -1)) {
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
        // Invia il segnale SIGALRM specificamente al thread delle syscalls
        pthread_kill(syscalls_thread, SIGALRM);
        
        // Cadenza l'invio dei segnali
        nanosleep(&ts, NULL);
    }
    
    free(name);
    return NULL;
}
#endif

int main() {

#ifndef ATTACK
    printf("Attacco stress disabilitato\n");
#else
    printf("Attacco stress abilitato\n");
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

    // Creazione dei due thread
    if (pthread_create(&syscalls_thread, NULL, syscalls_routine, NULL) != 0) {
        perror("Errore creazione Thread 1");
        return 1;
    }
    
#ifdef ATTACK
    if (pthread_create(&alrm_thread, NULL, alarm_routine, NULL) != 0) {
        perror("Errore creazione Thread 2");
        return 1;
    }
#endif

    // Attesa della terminazione dei thread
    pthread_join(syscalls_thread, NULL);
#ifdef ATTACK
    pthread_join(alrm_thread, NULL);
#endif

    printf("Esecuzione terminata regolarmente.\n");
    return 0;
}