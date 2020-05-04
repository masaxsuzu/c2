#!/bin/bash

# for func call tests
cat <<EOF | gcc -xc -c -o ../tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int ret42(int x, int y) { return x*y; }
EOF

assert() {
  expected="$1"
  input="$2"

  # IO on shared dir is too slow.
  # ../ would be /workspaces/
  ./c2 "$input" > ../tmp.s
  cc -o ../tmp ../tmp.s ../tmp2.o
  ../tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'int main(){ return 0; }'
assert 42 'int main(){ return 42; }'
assert 21 'int main(){ return 5+20-4; }'
assert 41 'int main(){ return  12 + 34 - 5 ; }'
assert 8 'int main(){ return 4*2; }'
assert 33 'int main(){ return 99/3; }'
assert 128 'int main(){ return (128); }'
assert 6 'int main(){ return (1+2) * 4 / 2; }'
assert 100 'int main(){ return +100; }'
assert 100 'int main(){ return -100+200; }'

assert 0 'int main(){ return 0==1; }'
assert 1 'int main(){ return 42==42; }'
assert 1 'int main(){ return 0!=1; }'
assert 0 'int main(){ return 42!=42; }'

assert 1 'int main(){ return 0<1; }'
assert 0 'int main(){ return 1<1; }'
assert 0 'int main(){ return 2<1; }'
assert 1 'int main(){ return 0<=1; }'
assert 1 'int main(){ return 1<=1; }'
assert 0 'int main(){ return 2<=1; }'

assert 1 'int main(){ return 1>0; }'
assert 0 'int main(){ return 1>1; }'
assert 0 'int main(){ return 1>2; }'
assert 1 'int main(){ return 1>=0; }'
assert 1 'int main(){ return 1>=1; }'
assert 0 'int main(){ return 1>=2; }'

assert 1 'int main(){ int a; return a=1; }'
assert 4 'int main(){ int a; int b;a=b=2; return b+a; }'
assert 6 'int main(){ int z; z=2;return  z+z*(z); }'
assert 1 'int main(){ int aa0; aa0=1; return  aa0;}'
assert 1 'int main(){ int za1; za1=1; return  za1;}'
assert 101 'int main(){ int b9;b9=101; return b9;}'
assert 102 'int main(){ int aa;int b1;aa=1;b1=101;return aa+b1; }'

assert 1 'int main(){ return 1; }'
assert 2 'int main(){ int return1;return1 = 2; return1; }'

assert 10 'int main(){ if(1==1) return 10; }'
assert 200 'int main(){ if(0==1) return 10; return 200; }'
assert 1 'int main(){ if(0==0) if(1==1) return 1; return 200; }'
assert 200 'int main(){ if(0==0) if(1==100) return 1; return 200; }'
assert 123 'int main(){ if(0!=0) return 1; else return 123; }'
assert 2 'int main(){ if(0==0) if(1!=1) return 1; else return 2; else return 123; }'
assert 10 'int main(){ int i;i=0; while(i<10) i=i+1; return i; }'
assert 55 'int main(){ int i; int j;i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main(){ for (;;) return 3; return 5; }'
assert 3 'int main(){ {1; {2;} return 3;} }'
assert 55 'int main(){ int i; int j; i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 3 'int main(){ return ret3(); }'
assert 5 'int main(){ return ret5(); }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 42 'int main(){ return ret42(6,7); }'
assert 43 'int main() { return ret43(1,42); } int ret43(int x,int y) { return x+y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

assert 5 'int main() { int x; int *y;x=3; y=&x; *y=5; return x; }'
assert 7 'int main() { int x; int y;x=3; y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x; int y;x=3; y=5; *(&y-1)=7; return x; }'
assert 3 'int main() { int x; x=3; return *&x; }'
assert 3 'int main() { int x; x=3; int *y; y=&x; int **z; z=&y; return **z; }'
assert 5 'int main() { int x; x=3; int y; y=5; return *(&x+1); }'
assert 5 'int main() { int x; x=3; int y; y=5; return *(1+&x); }'
assert 3 'int main() { int x; x=3; int y; y=5; return *(&y-1); }'
assert 5 'int main() { int x; x=3; int y; y=5; int *z; z=&x; return *(z+1); }'
assert 3 'int main() { int x; x=3; int y; y=5; int *z; z=&y; return *(z-1); }'
assert 8 'int main() { int x; x=3; int y; y=5; return foo(&x, y); } int foo(int *x, int y) { return *x + y; }'
assert 0 'int main() { int x; x=3; return &x-&x; }'
assert 2 'int main() { int x; x=3; return &x+2-&x; }'

assert 8 'int main() { return sizeof(1);}'
assert 8 'int main() { return sizeof 1 ;}'
assert 8 'int main() { int x; return sizeof(&x);}'
assert 32 'int main() { int x[4]; return sizeof(x);}'

assert 3 'int main() { int x[2]; int *y; y=&x; *y=3; return *x; }'
assert 3 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }'

echo OK