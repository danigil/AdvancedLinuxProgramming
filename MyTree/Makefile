.PHONY: all
all: stree

CC = gcc
CFLAGS = -Wall -g

stree: stree.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	-rm stree