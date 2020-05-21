CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard ./src/*.c)
OBJS=$(SRCS:.c=.o)

test-gen2: test c2-gen2 extern.o
	./c2-gen2 ./tests/test.c > tmp.s
	gcc -static -o ./tmp ./tmp.s extern.o
	./tmp

test: c2 extern.o
	./c2 ./tests/test.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s extern.o
	./tmp

c2-gen2: c2 $(SRCS) src/c2.h
	bash self.sh

extern.o: tests/extern.c
	gcc -xc -c -o extern.o tests/extern.c

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): ./src/c2.h

clean:
	rm -rf c2 c2-gen* *.o ./src/*.o *~ tmp*

fmt:
	clang-format src/*.c -i

debug: c2
	./c2 ./tests/debug.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s
	./tmp

.PHONY: test fmt clean debug
