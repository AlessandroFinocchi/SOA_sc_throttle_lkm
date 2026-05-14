CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include/devices


all:
	make single stress stress2 stresses stresses2 perf

clean:
	rm -f single stress stress2 stresses stresses2 perf

single: probe_single.c
	$(CC) $(CFLAGS) -o $@ $<

stress: probe_stress.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

stress2: probe_stress.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread -DATTACK

stresses: probe_stresses.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

stresses2: probe_stresses.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread -DATTACK

# da eseguire come root
perf: probe_perf.c
	$(CC) -O2 $(CFLAGS) -o $@ $< -lm