CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices

test_ioctl: test_ioctl.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_ioctl