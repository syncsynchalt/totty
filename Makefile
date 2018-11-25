CFLAGS = -Wall -D_XOPEN_SOURCE=700

all: totty

totty: totty.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o
	rm -f totty

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
