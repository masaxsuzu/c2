CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

test: c2
	./test.sh

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): c2.h

fmt:
	clang-format *.c -i

clean:
	rm -f c2 *.o *~ tmp*

.PHONY: test fmt clean
