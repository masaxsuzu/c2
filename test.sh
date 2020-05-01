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

assert 0 'main(){ return 0; }'
assert 42 'main(){ return 42; }'
assert 21 'main(){ return 5+20-4; }'
assert 41 'main(){ return  12 + 34 - 5 ; }'
assert 8 'main(){ return 4*2; }'
assert 33 'main(){ return 99/3; }'
assert 128 'main(){ return (128); }'
assert 6 'main(){ return (1+2) * 4 / 2; }'
assert 100 'main(){ return +100; }'
assert 100 'main(){ return -100+200; }'

assert 0 'main(){ return 0==1; }'
assert 1 'main(){ return 42==42; }'
assert 1 'main(){ return 0!=1; }'
assert 0 'main(){ return 42!=42; }'

assert 1 'main(){ return 0<1; }'
assert 0 'main(){ return 1<1; }'
assert 0 'main(){ return 2<1; }'
assert 1 'main(){ return 0<=1; }'
assert 1 'main(){ return 1<=1; }'
assert 0 'main(){ return 2<=1; }'

assert 1 'main(){ return 1>0; }'
assert 0 'main(){ return 1>1; }'
assert 0 'main(){ return 1>2; }'
assert 1 'main(){ return 1>=0; }'
assert 1 'main(){ return 1>=1; }'
assert 0 'main(){ return 1>=2; }'

assert 1 'main(){ return a=1; }'
assert 4 'main(){ a=b=2; return b+a; }'
assert 6 'main(){ z=2;return  z+z*(z); }'
assert 1 'main(){ aa0=1; return  aa0;}'
assert 1 'main(){ za1=1; return  za1;}'
assert 101 'main(){ b9=101; return b9;}'
assert 102 'main(){ aa=1;b1=101;return aa+b1; }'

assert 1 'main(){ return 1; }'
assert 2 'main(){ return1 = 2; return1; }'

assert 10 'main(){ if(1==1) return 10; }'
assert 200 'main(){ if(0==1) return 10; return 200; }'
assert 1 'main(){ if(0==0) if(1==1) return 1; return 200; }'
assert 200 'main(){ if(0==0) if(1==100) return 1; return 200; }'
assert 123 'main(){ if(0!=0) return 1; else return 123; }'
assert 2 'main(){ if(0==0) if(1!=1) return 1; else return 2; else return 123; }'
assert 10 'main(){ i=0; while(i<10) i=i+1; return i; }'
assert 55 'main(){ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'main(){ for (;;) return 3; return 5; }'
assert 3 'main(){ {1; {2;} return 3;} }'
assert 55 'main(){ i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 3 'main(){ return ret3(); }'
assert 5 'main(){ return ret5(); }'
assert 42 'main(){ return ret42(6,7); }'

assert 32 'main() { return ret32(); } ret32() { return 32; }'

echo OK