CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices

perm: test_ioctl_permissions.c
	$(CC) $(CFLAGS) -o $@ $<

ops: test_ioctl_operations.c
	$(CC) $(CFLAGS) -o $@ $<

state: test_ioctl_monitor_state.c
	$(CC) $(CFLAGS) -o $@ $<

prof: test_ioctl_profiling.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f perm ops state prof