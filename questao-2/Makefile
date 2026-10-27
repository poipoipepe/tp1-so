CC=gcc
CFLAGS=-Wall -Wextra -O2
LDLIBS=-lrt

all: A B A2 B2 limpa

%: %.c comum.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f A B A2 B2 limpa
	rm -f /dev/mqueue/fila-ligia-pedro-*

.PHONY: all clean
