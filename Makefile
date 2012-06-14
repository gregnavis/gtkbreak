CFLAGS=-std=c99 -W -Wall -Werror -pedantic `pkg-config --libs --cflags gtk+-2.0`

BINARY=gtkbreak

$(BINARY): $(BINARY).c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm $(BINARY)
