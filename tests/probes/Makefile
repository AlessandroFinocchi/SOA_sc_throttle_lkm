CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices

single: probe_single.c
	$(CC) $(CFLAGS) -o $@ $<

stress: probe_stress.c
	$(CC) $(CFLAGS) -o $@ $<

stresses: probe_stresses.c
	$(CC) $(CFLAGS) -o $@ $<

# da eseguire come root
perf: probe_perf.c
	$(CC) -O2 $(CFLAGS) -o $@ $< -lm

clean:
	rm -f probe_single probe_stress probe_stresses probe_perf