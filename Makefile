CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

clean: test
	rm -f c2 *.o *~ ../tmp*

test: c2
	./test.sh

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): c2.h

fmt:
	clang-format *.c -i

.PHONY: test fmt clean
