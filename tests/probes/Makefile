CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices

single: probe_single.c
	$(CC) $(CFLAGS) -o $@ $<

stress: probe_stress.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f ops perm