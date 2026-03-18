/* Si vuole testare che ioctl sia invocabile solo da root */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "sctrt_dev_ioctl.h"

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

    // 3. Invocazione della system call ioctl (impostiamo stato ON)
    param.data.new_state = true;
    if (ioctl(fd, SC_THROTTLE_SET_STATE, &param) < 0) {
        fprintf(stderr, "Errore ioctl: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    // 4. Rilascio delle risorse
    close(fd);

    // 5. Declassamento utente
    if (seteuid(1000) != 0) {
        fprintf(stderr, "Errore fatale durante il privilege dropping: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    //6. Verifica formale dell'avvenuto declassamento
    if (geteuid() == 0) {
        fprintf(stderr, "Errore critico di sicurezza: privilege dropping fallito.\n");
        return EXIT_FAILURE;
    }

    // 7. Acquisizione del file descriptor
    fd = open("/dev/sctrt_dev", O_RDWR);
    if (fd > 0) {
        fprintf(stderr, "Errore fatale open non-root level: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fd = open("/dev/sctrt_dev", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Errore fatale open non-root level: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // 8. Invocazione della system call w_ioctl
    if (ioctl(fd, SC_THROTTLE_SET_STATE, &param) != -EPERM) {
        fprintf(stderr, "Errore w_ioctl non-root level: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    // 9. Invocazione della system call r_ioctl
    if (ioctl(fd, SC_THROTTLE_GET_TELEM, &param) < 0) {
        fprintf(stderr, "Errore r_ioctl non-root level: %s\n", strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    printf("Risultato test: Successo\n");

    return EXIT_SUCCESS;
}