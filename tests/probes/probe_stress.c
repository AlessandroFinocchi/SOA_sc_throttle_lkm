/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations, eseguire
 * probe_stress per controllare se si è entrati nella kprobe.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdatomic.h>

// Variabile atomica per coordinare la terminazione del ciclo nel secondo thread
atomic_bool keep_running = ATOMIC_VAR_INIT(true);

// Identificatore del primo thread, globale affinché il secondo possa referenziarlo
pthread_t thread1;

void alarm_handler() {
    // L'handler è intenzionalmente vuoto.
    // Il kernel eseguirà il signal delivery interrompendo il thread bersaglio.
    fflush(stdout);
}

void* syscalls_routine(void* arg) {
    char* name = NULL;

    // asprintf allocates memory and points thread_name_ptr to it
    // It returns the number of bytes printed, or -1 on allocation failure.    
    if ((asprintf(&name, "%s", "stress") == -1)) {
        // asprintf leaves the pointer undefined on failure, ensure it is NULL
        return NULL;
    }

    // Imposta il nome del thread per il kernel (current->comm)
    pthread_setname_np(pthread_self(), name);

    int duration = *(int*)arg;
    int ms_sleep = 50;
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        // getpid() restituisce il TGID (Thread Group ID), che è condiviso.
        pid_t pid = getpid();
        
        printf("Thread [%s] - PID [%d] - chiamata PID numero %d\n", name, pid, i);
        fflush(stdout);
        
        // Attesa: la funzione nanosleep verrà interrotta dal segnale SIGALRM, 
        // ritornando prematuramente con errno = EINTR.
        nanosleep(&ts, NULL);
    }
    
    // Al termine delle iterazioni, segnala al secondo thread di arrestarsi
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
        pthread_kill(thread1, SIGALRM);
        
        // Cadenza l'invio dei segnali
        nanosleep(&ts, NULL);
    }
    
    free(name);
    return NULL;
}

int main() {
    pthread_t thread2;
    int ms_sleep = 50;
    int duration = 10 * 1000 / ms_sleep; /* stimato per circa 10 secondi */

    // Gli handler (signal disposition) sono condivisi a livello di processo.
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Nessun flag SA_RESTART: vogliamo che il segnale interrompa eventuali syscall
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Errore nella registrazione di sigaction");
        return 1;
    }

    // Creazione dei due thread
    if (pthread_create(&thread1, NULL, syscalls_routine, &duration) != 0) {
        perror("Errore creazione Thread 1");
        return 1;
    }
    
    if (pthread_create(&thread2, NULL, alarm_routine, NULL) != 0) {
        perror("Errore creazione Thread 2");
        return 1;
    }

    // Attesa della terminazione dei thread
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Esecuzione terminata regolarmente.\n");
    return 0;
}