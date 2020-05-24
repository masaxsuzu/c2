CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard ./src/*.c)
OBJS=$(SRCS:.c=.o)

full: test-gen1 test-gen2 test-gen3

test-gen3: c2-gen2 c2-gen3
	diff c2-gen2 c2-gen3
	echo Done test-gen3

test-gen2: c2-gen2 extern.o
	./c2-gen2 ./tests/test.c > tmp.s
	gcc -static -o ./tmp ./tmp.s extern.o
	./tmp
	echo Done test-gen2

test-gen1: c2-gen1 extern.o
	./c2-gen1  ./tests/test.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s extern.o
	./tmp
	echo Done test-gen1

c2-gen2: c2-gen1  $(SRCS) src/c2.h
	bash self.sh c2-gen1  c2-gen2

c2-gen3: c2-gen2 $(SRCS) src/c2.h
	bash self.sh c2-gen2 c2-gen3

c2-gen1: $(OBJS)
	$(CC) -o c2-gen1  $(OBJS) $(LDFLAGS)

extern.o: tests/extern.c
	gcc -xc -c -o extern.o tests/extern.c
$(OBJS): ./src/c2.h

test-gen1-win: c2-gen1-win.exe
	.\c2-gen1-win.exe .\tests\win.c > .\win\win.s
	gcc -static -o .\win\tmp .\win\win.s
	powershell .\test-win.ps1

c2-gen1-win.exe:
	powershell .\build-win.ps1

win\extern.obj: tests\extern.c
	gcc -xc -c -o .\win\extern.obj tests\extern.c

clean:
	rm -rf c2-gen* *.o ./src/*.o *~ tmp*

clean-win:
	powershell rm  -Force -ErrorAction Ignore .\win\*.s
	powershell rm  -Force -ErrorAction Ignore .\win\*.obj
	powershell rm  -Force -ErrorAction Ignore .\c2-gen1-win.exe

fmt:
	clang-format src/*.c -i

debug: c2-gen1 
	./c2-gen1  ./tests/debug.c > ./tmp.s
	gcc -static -o ./tmp ./tmp.s
	./tmp

diff:
	./c2-gen1  ${f} > 1.s
	${xcc} ${f} > 2.s
	diff 1.s 2.s

.PHONY: fmt clean debug diff test-gen1 test-gen2 test-gen3
