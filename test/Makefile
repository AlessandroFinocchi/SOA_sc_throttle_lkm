CC = gcc
CFLAGS = -Wall -Wextra -g -I../include/devices
LDFLAGS =

TARGET = test_ioctl
SRCS = test_ioctl.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)