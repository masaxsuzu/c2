CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard ./src/*.c)
OBJS=$(SRCS:.c=.o)

test: c2
	./c2 ./tests/test.c > ./tmp.s
	echo 'int only_decl(int x) { return x; }' | gcc -o tmp2.o -xc -c -
	gcc -static -o ./tmp ./tmp.s ./tmp2.o
	./tmp

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): ./src/c2.h

clean:
	rm -f c2 ./src/*.o *~ tmp*

fmt:
	clang-format src/*.c -i

debug: c2
	./c2 ./tests/debug.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s
	./tmp

.PHONY: test fmt clean debug
