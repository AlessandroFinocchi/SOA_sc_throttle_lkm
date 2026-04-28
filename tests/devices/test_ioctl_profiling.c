/* Si vuole testare le operazioni di ioctl */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <asm/unistd.h>        // For __NR_* macros (i.e. __NR_read)

#include "sctrt_dev_ioctl.h"

int print_conf(int fd);

int main() {
    // 1. Validazione rigorosa del contesto di esecuzione (EUID)
    if (geteuid() != 0) {
        fprintf(stderr, "Errore critico: permessi insufficienti .\n");
        fprintf(stderr, "Il programma deve essere eseguito con privilegi di root (EUID = 0).\n");
        fprintf(stderr, "EUID corrente: %d\n", geteuid());
        return EXIT_FAILURE;
    }

    // Da questo punto in poi, EUID == 0 è garantito.
    int fd;
    struct sc_throttle_param param = {0};

    // 2. Acquisizione del file descriptor
    fd = open("/dev/sctrt_dev", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Errore fatale open root level: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if(print_conf(fd) < 0) goto err;
    
    // 3. Invocazione della system call ioctl
    for(int i = 0; i < 20; i++) {
        if (ioctl(fd, SC_THROTTLE_GET_METRICS, &param) < 0) {
            fprintf(stderr, "Errore durante l'operazione ioctl (SC_THROTTLE_GET_METRICS): %s\n", strerror(errno));
            close(fd);
            return EXIT_FAILURE;
        }

        // 4. Stampa formattata delle metriche acquisite
        printf("==============================================\n");
        printf("      PROFILAZIONE SOTTOSISTEMA THROTTLING    \n");
        printf("==============================================\n");
        
        printf("--------------------------------------------------\n");
        printf("Picco thread bloccati/sec:    %u\n", param.data.profiler.peak_blocked_threads);
        printf("Totale thread bloccati   :    %lu\n", param.data.profiler.sum_blocked_threads);
        printf("Totale thread campioni   :    %u\n", param.data.profiler.total_samples);    
        printf("==================================================\n");

        printf("Ritardo massimo registrato:   %lu ns\n", param.data.profiler.peak_delay_ns);
        
        /* Gestione condizionale dell'output per evitare letture inconsistenti su dati non ancora campionati */
        if (param.data.profiler.peak_delay_ns > 0) {
            printf("Eseguibile resp. del picco:   %s\n", param.data.profiler.peak_prog_name);
            printf("EUID resp. del picco:         %u\n", param.data.profiler.peak_uid);
        } else {
            printf("Eseguibile resp. del picco:   N/A (Nessun throttling effettuato)\n");
            printf("EUID resp. del picco:         N/A\n");
        }
        
        sleep(1);
    }


    if(print_conf(fd) < 0) goto err;

    // 4. Rilascio delle risorse
    close(fd);

    printf("Risultato test: Successo\n");

    return EXIT_SUCCESS;

err:
    fprintf(stderr, "Errore ioctl: %s\n", strerror(errno));
    close(fd);
    return EXIT_FAILURE;
}

int print_conf(int fd) {
    int status;
    if ((status=ioctl(fd, SC_THROTTLE_PRINT_STATE, NULL) < 0)) return status;
    if ((status=ioctl(fd, SC_THROTTLE_PRINT_RATE, NULL) < 0))  return status;
    if ((status=ioctl(fd, SC_THROTTLE_PRINT_SYSCS, NULL) < 0)) return status;
    if ((status=ioctl(fd, SC_THROTTLE_PRINT_USERS, NULL) < 0)) return status;
    if ((status=ioctl(fd, SC_THROTTLE_PRINT_PROGS, NULL) < 0)) return status;

    return 0;
}