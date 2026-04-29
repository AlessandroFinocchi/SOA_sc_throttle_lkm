/* Si vuole testare l'accesione / spegnimento del monitor */

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

int main(int argc, char *argv[]) {
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
    
    // 3. Invocazione della system call ioctl (impostiamo lo stato in base all'argomento)
    param.data.new_state = (argc > 1) ? (atoi(argv[1]) != 0) : false;
    if (ioctl(fd, SC_THROTTLE_SET_STATE, &param) < 0) goto err;

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

    return 0;
}