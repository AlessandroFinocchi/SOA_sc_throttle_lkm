/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations, eseguire
 * probe_stress per controllare se si è entrati nella kprobe 
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {
    int uid;
    float milliseconds = 500;
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;

    for (int i = 0; i < 1200; i++) {
        uid = getpid();
        printf("%d\n", uid);
        nanosleep(&ts, NULL);
    }
}