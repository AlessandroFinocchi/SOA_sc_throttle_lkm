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
    
    // 3. Invocazione della system call ioctl (impostiamo stato ON)
    param.data.new_state = true;
    if (ioctl(fd, SC_THROTTLE_SET_STATE, &param) < 0) goto err;

    param.data.max_rate = 10;
    if (ioctl(fd, SC_THROTTLE_SET_RATE, &param) < 0) goto err;

    param.data.syscall_num = __NR_getpid;
    if (ioctl(fd, SC_THROTTLE_REG_SYS, &param) < 0) goto err;

    param.data.uid = 1000;
    if (ioctl(fd, SC_THROTTLE_REG_UID, &param) < 0) goto err;

    param.data.uid = 1001;
    if (ioctl(fd, SC_THROTTLE_REG_UID, &param) < 0) goto err;

    strncpy(param.data.prog_name, "single", MAX_PROG_NAME_LEN - 1);
    if (ioctl(fd, SC_THROTTLE_REG_PROG, &param) < 0) goto err;

    strncpy(param.data.prog_name, "stress", MAX_PROG_NAME_LEN - 1);
    if (ioctl(fd, SC_THROTTLE_REG_PROG, &param) < 0) goto err;

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