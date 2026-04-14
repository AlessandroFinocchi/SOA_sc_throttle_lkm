#include <stdio.h>
#include <unistd.h>

int main() {
    int uid = getpid();
    printf("%d\n", uid);
}