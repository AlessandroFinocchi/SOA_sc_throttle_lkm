/* 
 * Dopo aver lanciato tests/devices/test_ioctl_operations, eseguire
 * probe1 per controllare se si è entrati nella kprobe 
*/

#include <stdio.h>
#include <unistd.h>

int main() {
    int uid = getpid();
    printf("%d\n", uid);
}