CFLAGS=-std=c11 -g -static

test: c2
	./test.sh

c2: main.o
	$(CC) -o $@ $? $(LDFLAGS)

fmt:
	clang-format *.c -i

clean:
	rm -f c2 *.o *~ tmp*

.PHONY: test fmt clean
