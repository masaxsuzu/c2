#include <stdarg.h>

// -*- c -*-
int ext1;
int *ext2;

int ext1; int *ext2; 
int only_decl(int x) { return x; } 
int static_fn() { return 5; }
int false_fn() { return 512; }
int true_fn() { return 513; }

int sub_from_last(int x, int y, int z, int w) {
  return w- z - y - x;
}

int add_all1(int n, int x, ...) {
  va_list ap;
  va_start(ap, x);
  printf("x = %d\n",x);
  for( int i = 0; i < n; ++i ) {
    int y = va_arg(ap, int);
    printf("y = %d\n",y);
    x += y;
  }
  return x;
}

int add_all3(int n, int x, int y, ...) {
  va_list ap;
  va_start(ap, y);
  printf("x = %d\n",x);
  printf("y = %d\n",y);
  x = x + y;

  for( int i = 0; i < n; ++i ) {
    int z = va_arg(ap, int);
    printf("z = %d\n",z);
    x += z;
  }
  return x;
}