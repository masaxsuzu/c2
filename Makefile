CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

test: c2
	./c2 ./tests/test.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s
	./tmp

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): c2.h

clean:
	rm -f c2 *.o *~ tmp*

fmt:
	clang-format *.c -i

debug: c2
	./c2 ./tests/debug.c

.PHONY: test fmt clean debug
