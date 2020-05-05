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
  rm ../tmp.test.input -f
  echo "$input" | tee > ../tmp.test.input
  ./c2 ../tmp.test.input > ../tmp.s
  gcc -static -o ../tmp ../tmp.s ../tmp2.o
  if [ $? -ne 0 ]; then
    echo "compile error"
    exit 1
  fi
  ../tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 1 'int main() { int x = 1; {int x = 2;} return x;}'
assert 2 'int main() { int x = 1; {int x = 2; return x;} return x;}'
assert 3 'int x; int main() { x = 1; { x =3;} return x;}'

assert 1 'int funcinfunc(int x) { return x; } int f(int x, int y) { if (x==y) return funcinfunc(x); return 1; } int main() { return f(1,2); }'

assert 1 'int main() { char x = 1; return sizeof(x); }'
assert 4 'int main() { char x[4]; return sizeof(x); }'
assert 2 'int main() { char x = 2; return x; }'
assert 8 'char x; int main() { x = 8; return 8; }'
assert 3 'char x; int main() { x = 8; return sub(x, 5); } int sub(char a, char b) {return a-b;}'

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

assert 3 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return *(x+2); }'

assert 0 'int main() { int x[2][3]; int *y; y=x; y[0]=0; return x[0][0]; }'
assert 1 'int main() { int x[2][3]; int *y; y=x; y[1]=1; return x[0][1]; }'
assert 2 'int main() { int x[2][3]; int *y; y=x; y[2]=2; return x[0][2]; }'
assert 3 'int main() { int x[2][3]; int *y; y=x; y[3]=3; return x[1][0]; }'
assert 4 'int main() { int x[2][3]; int *y; y=x; y[4]=4; return x[1][1]; }'
assert 5 'int main() { int x[2][3]; int *y; y=x; y[5]=5; return x[1][2]; }'
assert 6 'int main() { int x[2][3]; int *y; y=x; y[6]=6; return x[2][0]; }'

assert 1 'int main() { int x = 1; return x;}'
assert 10 'int main() { int x = 10; int *y = &x; return *y;}'

assert 10 'int z; int main() { int x = 10; int *y = &x; return *y;}'
assert 20 'int z; int main() { z = 20; int x = 10; return z;}'
assert 3 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3]; }'
assert 32 'int x[4]; int main() { return sizeof(x); }'

echo OK