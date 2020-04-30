#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  # IO on shared dir is too slow.
  # ../ would be /workspaces/
  ./c2 "$input" > ../tmp.s
  cc -o ../tmp ../tmp.s
  ../tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 8 "4*2;"
assert 33 "99/3;"
assert 128 "(128);"
assert 6 "(1+2) * 4 / 2;"
assert 100 "+100;"
assert 100 "-100+200;"

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 1 'a=1;'
assert 4 'a=b=2;b+a;'
assert 6 'z=2;z+z*(z);'
assert 1 'aa0=1;'
assert 1 'za1=1;'
assert 101 'b9=101;'
assert 102 'aa=1;b1=101;aa+b1;'

assert 1 'return 1;'
assert 2 'return1 = 2; return1;'

assert 10 'if(1==1) return 10;'
assert 200 'if(0==1) return 10; 200;'
assert 1 'if(0==0) if(1==1) return 1; 200;'
assert 200 'if(0==0) if(1==100) return 1; 200;'
assert 123 'if(0!=0) 1; else 123;'
assert 2 'if(0==0) if(1!=1) 1; else 2; else 123;'
assert 10 'i=0; while(i<10) i=i+1; return i;'
assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert 3 'for (;;) return 3; return 5;'

echo OK