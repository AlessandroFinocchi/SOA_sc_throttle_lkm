/* Si vogliono testare le operazioni di profilazione */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <asm/unistd.h>        // For __NR_* macros (i.e. __NR_read)

#include "sctrt_dev_ioctl.h"

int print_conf(int fd);
char* nanoseconds_to_string(uint64_t ns);

int main() {
    int fd;
    struct sc_throttle_param param = {0};

    // 2. Acquisizione del file descriptor
    fd = open("/dev/sctrt_dev", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Errore fatale open root level: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if(print_conf(fd) < 0) goto err;
    
    // 3. Invocazione della system call ioctl
    for(int i = 0; i < 20; i++) {
        if(i != 0) 
            printf("\033[11A\r");
            
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

        printf("Ritardo massimo registrato:   %s\n", nanoseconds_to_string(param.data.profiler.peak_delay_ns));
        
        /* Gestione condizionale dell'output per evitare letture inconsistenti su dati non ancora campionati */
        if (param.data.profiler.peak_delay_ns > 0) {
            printf("Eseguibile resp. del picco:   %s\n", param.data.profiler.peak_prog_name);
            printf("EUID resp. del picco:         %u\n", param.data.profiler.peak_uid);
        } else {
            printf("Eseguibile resp. del picco:   N/A\n");
            printf("EUID resp. del picco:         N/A\n");
        }
        
        sleep(1);
    }


    if(print_conf(fd) < 0) goto err;

    // 4. Rilascio delle risorse
    close(fd);

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

/*
 * Converte un valore in nanosecondi in una stringa formattata.
 * Il chiamante è responsabile della deallocazione della memoria (free).
 */
char* nanoseconds_to_string(uint64_t ns) {
    const uint64_t NS_PER_US  = 1000ULL;
    const uint64_t NS_PER_MS  = 1000000ULL;
    const uint64_t NS_PER_SEC = 1000000000ULL;
    const uint64_t NS_PER_MIN = 60000000000ULL;

    // Calcolo delle unità temporali tramite divisioni e resti
    uint64_t mins = ns / NS_PER_MIN;
    ns %= NS_PER_MIN;

    uint64_t secs = ns / NS_PER_SEC;
    ns %= NS_PER_SEC;

    uint64_t ms = ns / NS_PER_MS;
    ns %= NS_PER_MS;

    uint64_t mus = ns / NS_PER_US;
    uint64_t remaining_ns = ns % NS_PER_US;

    size_t buffer_size = 128;
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        return NULL; // Gestione del fallimento di allocazione
    }

    snprintf(buffer, buffer_size, 
             "%" PRIu64 " min, %" PRIu64 " s, %" PRIu64 " ms, %" PRIu64 " \xC2\xB5s, %" PRIu64 " ns",
             mins, secs, ms, mus, remaining_ns);

    return buffer;
}