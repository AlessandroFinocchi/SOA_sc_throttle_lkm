CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices

ops: test_ioctl_operations.c
	$(CC) $(CFLAGS) -o $@ $<

state: test_ioctl_monitor_state.c
	$(CC) $(CFLAGS) -o $@ $<


perm: test_ioctl_permissions.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f ops perm