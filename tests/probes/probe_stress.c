/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations, eseguire
 * probe_stress per controllare se si è entrati nella kprobe 
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {
    int uid;
    int ms_sleep = 1000;
    // int duration = 1 * 60 * 1000 / ms_sleep; /* il programma dura 1 minuto */
    int duration = 10 * 1000 / ms_sleep; /* il programma dura 10 secondi */
    struct timespec ts;
    
    ts.tv_sec = ms_sleep / 1000;
    ts.tv_nsec = (ms_sleep % 1000) * 1000000L;

    for (int i = 0; i < duration; i++) {
        uid = getpid();
        printf("%d\n", uid);
        nanosleep(&ts, NULL);
    }

    printf("%ld\n", ts.tv_nsec);
}