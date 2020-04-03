CFLAGS=-std=c11 -g -static

test: c2
	./test.sh

c2: main.o
	$(CC) -o $@ $? $(LDFLAGS)

clean:
	rm -f c2 *.o *~ tmp*

.PHONY: test clean
