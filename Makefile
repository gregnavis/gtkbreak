CFLAGS=-std=c99 -W -Wall -pedantic `pkg-config --libs --cflags gtk+-2.0`

BINARY=break

$(BINARY): $(BINARY).c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm $(BINARY)