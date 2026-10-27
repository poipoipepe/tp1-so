CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
LDLIBS  = -lpthread -lm

pi_threads: pi_threads.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f pi_threads

.PHONY: clean
