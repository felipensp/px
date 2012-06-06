CC=gcc
CFLAGS=-Wall -g
OBJECTS=main.o cmd.o trace.o

px: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(OBJECTS) px
