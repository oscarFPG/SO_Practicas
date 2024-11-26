CC = gcc
CFLAGS = -g -pthread
LDFLAGS = -g -pthread

all: disco

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

disco: disco.o
	$(CC) $(LDFLAGS) -o $@ $<

.PHONY: clean all


clean:
	-rm disco disco.o
