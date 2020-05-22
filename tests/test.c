long assert(long want, long got, char *code) {
    if (want != got) {
        printf("%s => %d expected but got %d\n", code, want, got);
        exit(1);
    }
    printf("%s => %ld\n", code, got);
    return 0;
}

int ret3() {
    return 3;
}
int ret42(int x, int y) {
    return x*y;
}
int fib(int n) {
    if (n <= 1) {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}

int fninfn(int x) { 
    return x; 
} 
int nested(int x, int y) { 
    if (x==y) 
        return fninfn(x); 
    return 1; 
}

// return -1; line comment

/*
    return 255; block comment
*/

int gx ;
char gy;
char gz;
int gw[4];

int sub(char a, char b) {return a-b;}
int foo(int *x, int y) { return *x + y; }


// this function would be linked.
int only_decl(int x);
int dec2(int x);
int decl(int x) {
    return decl2(x);
}
int decl2(int x) {
    return 2*x;
}

int sizeof_nested_type1(int *x[4]) { 
    // warning: 
    // sizeof on array function parameter will return size of 
    // 'int **' instead of 'int *[4]'
    return sizeof(x);
}

int sizeof_nested_type2(int (*x)[4]) { 
    return sizeof(x);
}

void nop() {
}

void nop2() {
  return;
}

typedef int MyInt;

static int static_fn() { return 3; }

int print_board(int (*board)[10]) {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++)
      if (board[i][j]) {
        printf("Q ");
      }
      else
        printf(". ");
      printf("\n");
  }
  printf("\n\n");
}

int conflict(int (*board)[10], int row, int col) {
  for (int i = 0; i < row; i++) {
    if (board[i][col])
      return 1;
    int j = row - i;
    if (0 < col - j + 1 && (board[i][col - j]))
        return 1;
    if (col + j < 10 && (board[i][col + j]))
        return 1;
  }
  return 0;
}

int solve(int (*board)[10], int row) {
  if (row > 9) {
    print_board(board);
    return 0;
  }
  for (int i = 0; i < 10; i++) {
    if (!conflict(board, row, i)) {
      board[row][i] = 1;
      solve(board, row + 1);
      board[row][i] = 0;
    }
  }
}

int param_decay(int x[]) { return x[0]; }

int global_voidfn;

void void_dec_global_val_2(void) {
  global_voidfn -= 1;  
}

void void_dec_global_val() {
  void_dec_global_val_2();
}

char g3 = 3;
short g4 = 4;
int g5 = 5;
long g6 = 6;
int *g7 = &g5;
char *g8 = "abc";
int g9[3] = {0, 1 };
char *g10[] = {"foo", "bar"};

struct {char a; int b;} g11[2] = {{1, 2}, {3, 4}};
struct {int a[2];} g12[2] = {{{1, 2}}};

struct {int a[2];} g13[2] = {{1, 2}, 3, 4};
struct {int a[2];} g14[2] = {1, 2, 3, 4};
char *g15 = {"foo"};
char g16[][4] = {'f', 'o', 'o', 0, 'b', 'a', 'r', 0};

char g17[] = "foobar";
char g18[10] = "foobar";
char g19[3] = "foobar";

char *g20 = g17+0;
char *g21 = g17+3;
char *g22 = &g17-3;

char *g23[] = {g17+0, g17+3, g17-3};
int g24=3;
int *g25=&g24;
int g26[3] = {1, 2, 3};
int *g27 = g26 + 1;

extern int ext1;
extern int *ext2;
static int ext3 = 3;

int;
struct {char a; int b;};
typedef struct {char a; int b;} Ty1;

int counter() {
    static int i;
    static int j = 1+1;
    return i++ + j++;
}

typedef struct Tree {
  int val;
  struct Tree *lhs;
  struct Tree *rhs;
} Tree;

Tree *tree = &(Tree){
  1,
  &(Tree){
    2,
    &(Tree){ 3, 0, 0 },
    &(Tree){ 4, 0, 0 },
  },
  0,
};

_Bool true_fn();
_Bool false_fn();

int main() {
    assert(0, 0, "0");
    assert(42, 42, "42");

    assert(21, 5 + 20 - 4, "5+20-4");
    assert(41, 12 + 34 - 5, "12 + 34 - 5");
    assert(8, 4 * 2, "4*2");
    assert(33, 99 / 3, "99/3");
    assert(128, (128), "(128)");
    assert(6, (1 + 2) * 4 / 2, "(1+2) * 4 / 2");
    assert(100, +100, "+100");
    assert(100, -100 + 200, "-100+200");

    assert(0, 0 == 1, "0==1");
    assert(1, 42 == 42, "42==42");
    assert(1, 0 != 1, "0!=1");
    assert(0, 42 != 42, "42!=42");

    assert(1, 0 < 1, "0 < 1");
    assert(0, 1 < 1, "1 < 1");
    assert(0, 2 < 1, "2 < 1");
    assert(1, 0 <= 1, "0 <= 1");
    assert(1, 1 <= 1, "1 <= 1");
    assert(0, 2 <= 1, "2 <= 1");

    assert(0, 0 > 1, "0 > 1");
    assert(0, 1 > 1, "1 > 1");
    assert(1, 2 > 1, "2 > 1");
    assert(0, 0 >= 1, "0 >= 1");
    assert(1, 1 >= 1, "1 >= 1");
    assert(1, 2 >= 1, "2 >= 1");

    assert(1, ({ int a; a=1;}), "({ int a; a=1;})");
    assert(4, ({ int a; int b;a=b=2; b+a;}),"({ int a; int b;a=b=2; b+a;})");
    assert(6, ({ int z; z=2; z+z*(z);}),"({ int z; z=2; z+z*(z);})");
    assert(1, ({ int aa0; aa0=1; aa0;}),"({ int aa0; aa0=1; aa0;})");
    assert(1, ({ int za1; za1=1; za1;}),"({ int za1; za1=1; za1;})");
    assert(101, ({ int b9;b9=101; b9;}),"({ int b9;b9=101; b9;})");
    assert(102, ({ int aa;int b1;aa=1;b1=101; aa+b1;}),"({ int aa;int b1;aa=1;b1=101; aa+b1;})");
    assert(10, ({ int x_a = 10; x_a;}), "int x_a = 10; x_a;");
    assert(10, ({ int A = 10; A;}), "int A = 10; A;");

    assert(10, ({ int a; if(1==1) a=10; a;}), "({ int a; if(1==1) a=10; a;})");
    assert(200,({ int a=200; if(0==1) a=10 ;a; }), "({ int a=200; if(0==1) a=10 ;a; })");
    assert(1,({ int a = 2;if(0==0) if(1==1) a=1; a; }), "({ int a = 2;if(0==0) if(1==1) a=1; a; })");
    assert(123,({ int a = 2;if(0!=0) a=1; else a=123; a;}), "({ int a = 2;if(0!=0) a=1; else a=123; a;})");

    assert(10,({ int i;i=0; while(i<10) i=i+1; i; }), "({ int i;i=0; while(i<10) i=i+1; i; })");
    assert(55,({ int i; int j;i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i; int j;i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");
    assert(3,({ 1; {2;} 3; }), "({ 1; {2;} 3; })");
    assert(55,({ int i; int j; i=0; j=0; while(i<=10) {j=i+j; i=i+1;} j; }), "({ int i; int j; i=0; j=0; while(i<=10) {j=i+j; i=i+1;} j; })");

    assert(5,({ int x; int *y;x=3; y=&x; *y=5; x; }), "({ int x; int *y;x=3; y=&x; *y=5; x; })");
    assert(7,({ int x; int y;x=3; y=5; *(&x+1)=7; y; }), "({ int x; int y;x=3; y=5; *(&x+1)=7; y; })");
    assert(7,({ int x; int y;x=3; y=5; *(&y-1)=7; x; }), "({ int x; int y;x=3; y=5; *(&y-1)=7; x; })");
    assert(3,({ int x; x=3; *&x; }), "({ int x; x=3; *&x; })");
    assert(3,({ int x; x=3; int *y; y=&x; int **z; z=&y; **z; }), "({ int x; x=3; int *y; y=&x; int **z; z=&y; **z; })");
    assert(5,({ int x; x=3; int y; y=5; *(&x+1); }), "({ int x; x=3; int y; y=5; *(&x+1); })");
    assert(5,({ int x; x=3; int y; y=5; *(1+&x); }), "({ int x; x=3; int y; y=5; *(1+&x); })");
    assert(3,({ int x; x=3; int y; y=5; *(&y-1); }), "({ int x; x=3; int y; y=5; *(&y-1); })");
    assert(5,({ int x; x=3; int y; y=5; int *z; z=&x; *(z+1); }), "({ int x; x=3; int y; y=5; int *z; z=&x; *(z+1); })");
    assert(3,({ int x; x=3; int y; y=5; int *z; z=&y; *(z-1); }), "({ int x; x=3; int y; y=5; int *z; z=&y; *(z-1); })");
    assert(8,({ int x; x=3; int y; y=5; foo(&x, y); }), "({ int x; x=3; int y; y=5; foo(&x, y); })");
    assert(0,({ int x; x=3; &x-&x; }), "({ int x; x=3; &x-&x; })");
    assert(2,({ int x; x=3; &x+2-&x; }), "({ int x; x=3; &x+2-&x; })");

    assert(4,({ sizeof(1);}), "({ sizeof(1);})");
    assert(4,({ sizeof 1 ;}), "({ sizeof 1 ;})");
    assert(8,({ int x; sizeof(&x);}), "({ int x; sizeof(&x);})");
    assert(16,({ int x[4]; sizeof(x);}), "({ int x[4]; sizeof(x);})");

    assert(3,({ int x[2]; int *y; y=&x; *y=3; *x; }), "({ int x[2]; int *y; y=&x; *y=3; *x; })");
    assert(3,({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; })");
    assert(4,({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); })");
    assert(5,({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); })");

    assert(3,({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })");
    assert(4,({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })");
    assert(5,({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");
    assert(5,({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");
    assert(5,({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); })");

    assert(0,({ int x[2][3]; int *y; y=x; y[0]=0; x[0][0]; }), "({ int x[2][3]; int *y; y=x; y[0]=0; x[0][0]; })");
    assert(1,({ int x[2][3]; int *y; y=x; y[1]=1; x[0][1]; }), "({ int x[2][3]; int *y; y=x; y[1]=1; x[0][1]; })");
    assert(2,({ int x[2][3]; int *y; y=x; y[2]=2; x[0][2]; }), "({ int x[2][3]; int *y; y=x; y[2]=2; x[0][2]; })");
    assert(3,({ int x[2][3]; int *y; y=x; y[3]=3; x[1][0]; }), "({ int x[2][3]; int *y; y=x; y[3]=3; x[1][0]; })");
    assert(4,({ int x[2][3]; int *y; y=x; y[4]=4; x[1][1]; }), "({ int x[2][3]; int *y; y=x; y[4]=4; x[1][1]; })");
    assert(5,({ int x[2][3]; int *y; y=x; y[5]=5; x[1][2]; }), "({ int x[2][3]; int *y; y=x; y[5]=5; x[1][2]; })");
    // assert(6,({ int x[2][3]; int *y; y=x; y[6]=6; x[2][0]; }), "({ int x[2][3]; int *y; y=x; y[6]=6; x[0][0]; })");

    assert(10, ({ int x = 10; int *y = &x; *y;}), "({ int x = 10; int *y = &x; *y;})");

    assert(20, ({gz = 20; int x = 10; gz;}), "({gz = 20; int x = 10; z;})");
    assert(3, ({ gw[0]=0; gw[1]=1; gw[2]=2; gw[3]=3; gw[3];}),"({ gw[0]=0; gw[1]=1; gw[2]=2; gw[3]=3; gw[3];})");
    assert(16, ({ sizeof(gw);}), "({ sizeof(gw);})");

    assert(3, ret3(), "ret3()");
    assert(42, ret42(7,6), "ret42(7,6)");
    assert(55, fib(9), "fib(9)");

    assert(1, nested(1,100), "nested(1,100)");
    assert(100, nested(100,100), "nested(100,100)");

    assert(97, "abc"[0], "\"abc\"[0]");
    assert(98, "abc"[1], "\"abc\"[1]");
    assert(99, "abc"[2], "\"abc\"[2]");

    assert(7,"\a"[0], "\"\\a\"[0]\"");
    assert(8,"\b"[0], "\"\\b\"[0]\"");
    assert(9,"\t"[0], "\"\\t\"[0]\"");
    assert(10,"\n"[0], "\"\\n\"[0]\"");
    assert(11,"\v"[0], "\"\\v\"[0]\"");
    assert(12,"\f"[0], "\"\\f\"[0]\"");
    assert(13,"\r"[0], "\"\\r\"[0]\"");
    assert(27,"\e"[0], "\"\\e\"[0]\"");
    assert(0,"\0"[0], "\"\\0\"[0]\"");
    assert(106,"\j"[0], "\"\\j\"[0]\"");
    assert(107,"\k"[0], "\"\\k\"[0]\"");
    assert(108,"\l"[0], "\"\\l\"[0]\"");

    assert(1,({ int x = 1; {int x = 2;} x;}), "{ int x = 1; {int x = 2;} x;}");
    assert(2,({ int x = 2; int y = ({int x = 1; x;}); x;}), "({ int x = 2; int y = ({int x = 1; x;}); x;})");
    assert(3,({ {gx = 3;} gx;}), "({ {gx = 3;} gx;})");
    
    assert(1,({ char x = 1; sizeof(x);}), "({ char x = 1; sizeof(x);})");
    assert(2,({ char x = 2; x;}), "({ char x = 2; x;})");
    assert(4,({ char x[4]; sizeof(x);}), "({ char x[4]; sizeof(x);})");
    assert(8,({ gy = 8; gy;}), "(global variable): ({ gy = 8; gy;})");
    assert(3,({ gy = 8; sub(gy,5);}), "(global variable): ({ gy = 8; sub(gy,5);})");

    assert(123, ({ 123; }), "({ 123;})");
    assert(3, ({ 1;2;3; }), "({ 1;2;3;})");
    assert(255, ({ int x = 255; x;}), "({ int x = 255; x;})");

    assert(10, ({ struct {int a; int b;} x; x.a = 10; x.a; }), "({ struct {int a; int b;} x; x.a = 10; x.a; })");
    assert(20, ({ struct {int a; int b;} x; x.b = 20; x.b; }), "({ struct {int a; int b;} x; x.b = 20; x.b; })");

    assert(1, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; }), "struct {int a; int b;} x; x.a=1; x.b=2; x.a;");
    assert(2, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; }), "struct {int a; int b;} x; x.a=1; x.b=2; x.b;");
    assert(1, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; }), "struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a;");
    assert(2, ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; }), "struct {char a; int b; char c;} a; x.b=x; x.a=2; x.b=3; x.b;");
    assert(3, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; }), "struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c;");

    assert(0, ({ struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a; }), "struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a;");
    assert(1, ({ struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b; }), "struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b;");
    assert(2, ({ struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a; }), "struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a;");
    assert(3, ({ struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b; }), "struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b;");

    assert(6, ({ struct {int a[3]; int b[5];} x; int *p=&x; x.a[0]=6; p[0]; }), "struct {int a[3]; int b[5];} x; int *p=&x; x.a[0]=6; p[0];");
    assert(7, ({ struct {int a[3]; int b[5];} x; int *p=&x; x.b[0]=7; p[3]; }), "struct {int a[3]; int b[5];} x; int *p=&x; x.b[0]=7; p[3];");

    assert(6, ({ struct { struct { int b; } a; } x; x.a.b=6; x.a.b; }), "struct { struct { int b; } a; } x; x.a.b=6; x.a.b;");

    assert(4, ({ struct {int a;} x; sizeof(x); }), "struct {int a;} x; sizeof(x);");
    assert(8, ({ struct {int a; int b;} x; sizeof(x); }), "struct {int a; int b;} x; sizeof(x);");
    assert(12, ({ struct {int a[3];} x; sizeof(x); }), "struct {int a[3];} x; sizeof(x);");
    assert(16, ({ struct {int a;} x[4]; sizeof(x); }), "struct {int a;} x[4]; sizeof(x);");
    assert(24, ({ struct {int a[3];} x[2]; sizeof(x); }), "struct {int a[3];} x[2]; sizeof(x)};");
    assert(2, ({ struct {char a; char b;} x; sizeof(x); }), "struct {char a; char b;} x; sizeof(x);");
    assert(8, ({ struct {char a; int b;} x; sizeof(x); }), "struct {char a; int b;} x; sizeof(x);");
    assert(8, ({ struct {int a; char b;} x; sizeof(x); }), "struct {int a; char b;} x; sizeof(x);");
    
    assert(7, ({ int x;  char y; int a=&x; int b=&y; b-a; }), "int  x; char y; int a=&x; int b=&y; b-a;");
    assert(1,  ({ char x;  int y; int a=&x; int b=&y; b-a; }), "char x; int  y; int a=&x; int b=&y; b-a;");

    assert(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); }), "struct t {int a; int b;} x; struct t y; sizeof(y);");
    assert(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); }), "struct t {int a; int b;}; struct t y; sizeof(y);");
    assert(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }), "struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y);");
    assert(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }), "struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x;");

    assert(3, ({ struct t {char a;} x; struct t *y = &x; x.a=3; y->a; }), "struct t {char a;} x; struct t *y = &x; x.a=3; y->a;");
    assert(3, ({ struct t {char a;} x; struct t *y = &x; y->a=3; x.a; }), "struct t {char a;} x; struct t *y = &x; y->a=3; x.a;");

    assert(1, ({ typedef int t; t x=1; x; }), "typedef int t; t x=1; x;");
    assert(1, ({ typedef struct {int a;} t; t x; x.a=1; x.a; }), "typedef struct {int a;} t; t x; x.a=1; x.a;");
    assert(1, ({ typedef int t; t t=1; t; }), "typedef int t; t t=1; t;");
    assert(2, ({ typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a; }), "typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a;");
    
    assert(1, ({ + + 1;}), "+ + 1");
    assert(10, ({ - + - 10;}), "- + - 10");

    assert(42, ({ decl(21);}), "decl(21);");
    assert(255, ({ only_decl(255);}), "only_decl(255);");

    assert( 255, ({ short x = 255; x;}), "short x = 255; x;");
    assert(2, ({ short x; sizeof(x);}), "short x; sizeof(x);");
    assert(8, ({ long x; sizeof(x);}), "long x; sizeof(x);");
    assert(2, ({ int y = 1; short x = 1; x+1;}), "int y = 1; short x = 1; x+1;");
    assert(2, ({ short x = 1;int y = 1; x+1;}), "short x = 1;int y = 1; x+1;");
    assert(2, ({ int y = 1; long x = 1; x+1;}), "int y = 1; long x = 1; x+1;");
    assert(2, ({ long x = 1;int y = 1; x+1;}), "long x = 1;int y = 1; x+1;");
    assert( 2147483647, ({ long x = 2147483647; x;}), "long x = 2147483647; x;");
    assert( 2147483649, ({ long x = 2147483647; long y = 2; x+y;}), "long x = 2147483647; long y = 2; x+y;");    
    assert( 1, ({ long x = -2147483649; long y = 2147483648; -x-y;}), "long x = 2147483649; long y = -2147483648; -x-y;");    

    assert(24, ({ int *x[3]; sizeof(x); }), "int *x[3]; sizeof(x);");
    assert(8, ({ int (*x)[3]; sizeof(x); }), "int (*x)[3]; sizeof(x);");
    assert(3, ({ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }), "int *x[3]; int y; x[0]=&y; y=3; x[0][0];");
    assert(4, ({ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }), "int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0];");
    assert(8, ({ int *x[4]; sizeof_nested_type1(x); }), "int *x[4]; sizeof_nested_type1(x);");
    assert(8, ({ int (*x)[4]; sizeof_nested_type2(x); }), "int (*x)[4]; sizeof_nested_type2(x);");
    assert(32, ({ int **x[4]; sizeof(x); }), "int **x[4]; sizeof(x); ");
    assert(8, ({ int *(*x)[4]; sizeof(x); }), "int *(*x)[4]; sizeof(x); ");
    assert(8, ({ int (**x)[4]; sizeof(x); }), "int (**x)[4]; sizeof(x); ");

    assert(123, ({ nop(); 123;}), "nop(); 123;");
    assert(255, ({ nop2(); 255;}), "nop(); 255;");

    assert(0, ({ _Bool x=0; x; }), "_Bool x=0; x;");
    assert(1, ({ _Bool x=1; x; }), "_Bool x=1; x;");
    assert(1, ({ _Bool x=2; x; }), "_Bool x=2; x;");
    assert(1, ({ _Bool x; sizeof(x);}), "_Bool x; sizeof(x);");

    assert(1, ({ sizeof(void); }), "sizeof(void);");
    assert(1, ({ sizeof(_Bool); }), "sizeof(_Bool);");
    assert(1, ({ sizeof(char); }), "sizeof(char);");
    assert(2, ({ sizeof(short); }), "sizeof(short);");
    assert(4, ({ sizeof(int); }), "sizeof(int);");
    assert(8, ({ sizeof(long); }), "sizeof(long);");

    assert(4, ({ sizeof(struct {int a;}); }), "sizeof(struct {int a;});");
    assert(8, ({ sizeof(int (**)); }), "sizeof(int (**));");
    assert(32, ({ sizeof(int **[4]); }), "sizeof(int **[4]);");

    assert(1, ({ char x; sizeof(x); }), "char x; sizeof(x);");
    assert(2, ({ short int x; sizeof(x); }), "short int x; sizeof(x);");
    assert(2, ({ int short x; sizeof(x); }), "int short x; sizeof(x);");
    assert(4, ({ int x; sizeof(x); }), "int x; sizeof(x);");
    assert(4, ({ typedef t; t x; sizeof(x); }), "typedef t; t x; sizeof(x);");
    assert(4, ({ typedef typedef t; t x; sizeof(x); }), "typedef typedef t; t x; sizeof(x);");
    assert(8, ({ long int x; sizeof(x); }), "long int x; sizeof(x);");
    assert(8, ({ int long x; sizeof(x); }), "int long x; sizeof(x);");
    assert(8, ({ long long x; sizeof(x); }), "long long x; sizeof(x);");
    assert(8, ({ long int long x; sizeof(x); }), "long int long x; sizeof(x);");

    assert(3, ({ MyInt x=3; x; }), "MyInt x=3; x;");

    assert(131585, (int)8590066177, "(int)8590066177");
    assert(513, (short)8590066177, "(short)8590066177");
    assert(1, (char)8590066177, "(char)8590066177");
    assert(1, (_Bool)1, "(_Bool)1");
    assert(1, (_Bool)2, "(_Bool)2");
    assert(0, (_Bool)(char)256, "(_Bool)(char)256");
    assert(1, (long)1, "(long)1");
    assert(0, (long)&*(int *)0, "(long)&*(int *)0");
    assert(5, ({ int x=5; long y=(long)&x; *(int*)y; }), "int x=5; long y=(long)&x; *(int*)y");

    assert(97, 'a', "'a'");
    assert(10, '\n', "\'\\n\'");

    assert(0, ({ enum { zero, one, two }; zero; }), "enum { zero, one, two }; zero;");
    assert(1, ({ enum { zero, one, two }; one; }), "enum { zero, one, two }; one;");
    assert(2, ({ enum { zero, one, two }; two; }), "enum { zero, one, two }; two;");
    assert(5, ({ enum { five=5, six, seven }; five; }), "enum { five=5, six, seven }; five;");
    assert(6, ({ enum { five=5, six, seven }; six; }), "enum { five=5, six, seven }; six;");
    assert(0, ({ enum { zero, five=5, three=3, four }; zero; }), "enum { zero, five=5, three=3, four }; zero;");
    assert(5, ({ enum { zero, five=5, three=3, four }; five; }), "enum { zero, five=5, three=3, four }; five;");
    assert(3, ({ enum { zero, five=5, three=3, four }; three; }), "enum { zero, five=5, three=3, four }; three;");
    assert(4, ({ enum { zero, five=5, three=3, four }; four; }), "enum { zero, five=5, three=3, four }; four;");
    assert(4, ({ enum { zero, one, two } x; sizeof(x); }), "enum { zero, one, two } x; sizeof(x);");
    assert(4, ({ enum t { zero, one, two }; enum t y; sizeof(y); }), "enum t { zero, one, two }; enum t y; sizeof(y);");
    
    assert(3, ({ static_fn(); }), "static_fn();");

    assert(4, ({ int i = 0; for(int j = 0; j < 5; j = j + 1) i = j; i; }), "int i = 0; for(int j = 0; j < 5; j = j + 1) i = j; i;");
    assert(10, ({ int i = 10; for(int i = 0; i < 5; i = i + 1) i = 4; i; }), "int i = 10; for(int i = 0; i < 5; i = i + 1) i = 4; i;");

    assert(0, ({
      int board[100]; 
      for (int i = 0; i < 100; i=i+1) 
        board[i] = 0; 
      solve(board, 0); 
      0;
    }), "nqueen");

    assert(3, (1,2,3), "1,2,3");

    assert(3, ({ int i=2; ++i; }), "int i=2; ++i;");
    assert(1, ({ int i=2; --i; }), "int i=2; --i;");
    assert(2, ({ int i=2; i++; }), "int i=2; i++;");
    assert(2, ({ int i=2; i--; }), "int i=2; i--;");
    assert(3, ({ int i=2; i++; i; }), "int i=2; i++; i;");
    assert(1, ({ int i=2; i--; i; }), "int i=2; i--; i;");
    assert(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++;");
    assert(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; ++*p; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; ++*p;");
    assert(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p--; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p--;");
    assert(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; --*p; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; --*p;");
    assert(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[0]; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++); a[0];");
    assert(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[1]; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++); a[0];");
    assert(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[2]; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++); a[0];");
    assert(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; *p; }), "int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++); a[0];");

    assert(0, !1, "!1");
    assert(0, !2, "!2");
    assert(1, !0, "!0");

    assert(7, ({ int i=2; i+=5; i; }), "int i=2; i+=5; i;");
    assert(7, ({ int i=2; i+=5; }), "int i=2; i+=5;");
    assert(3, ({ int i=5; i-=2; i; }), "int i=5; i-=2; i;");
    assert(3, ({ int i=5; i-=2; }), "int i=5; i-=2;");
    assert(6, ({ int i=3; i*=2; i; }), "int i=3; i*=2; i;");
    assert(6, ({ int i=3; i*=2; }), "int i=3; i*=2;");
    assert(3, ({ int i=6; i/=2; i; }), "int i=6; i/=2; i;");
    assert(3, ({ int i=6; i/=2; }), "int i=6; i/=2;");

    assert(-1, ~0, "~0");
    assert(0, ~-1, "~-1");

    assert(0, 0&&1, "0&&1");
    assert(0, (2-2)&&5, "(2-2)&&5");
    assert(1, 1&&5, "1&&5");

    assert(1, 0||1, "0||1");
    assert(1, (2-2)||5, "(2-2)||5");
    assert(0, 0||0, "0||0");

    assert(0, 0&0, "0&0");
    assert(0, 0&1, "0&1");
    assert(0, 1&0, "1&0");
    assert(1, 1&1, "1&1");
    assert(0, 0|0, "0|0");
    assert(1, 0|1, "0|1");
    assert(1, 1|0, "1|0");
    assert(1, 1|1, "1|1");
    assert(0, 0^0, "0^0");
    assert(1, 1^0, "1^0");
    assert(1, 0^1, "0^1");
    assert(0, 1^1, "1^1");

    // 11 = 001011
    // 56 = 111000
    
    //  8 = 001000
    // 59 = 111011
    // 51 = 110011
    assert(8,  11&56, "11&56");
    assert(59, 11|56, "11|56");
    assert(51, 11^56, "11^56");

    assert(3, ({ int i=0; for(;i<10;i++) { if (i == 3) break; } i; }), "int i=0; for(;i<10;i++) { if (i == 3) break; } i;");
    assert(4, ({ int i=0; while (1) { if (i++ == 3) break; } i; }), "int i=0; while { if (i == 3) break; } i;");
    assert(4, ({ int i=0; while (1) { while(1) break; if (i++ == 3) break; } i; }), "int i=0; while { if (i == 3) break; } i;");
    assert(3, ({int i=0; for(;i<10;i++) { for(;;) break; if (i == 3) break; } i;}), "int i=0; for(;i<10;i++) { for(;;) break; if (i == 3) break; } i;");

    assert(10, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } i; }), "int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } i;");
    assert(6, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } j; }), "int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } j;");
    assert(10, ({ int i=0; int j=0; for(;!i;) { for (;j!=10;j++) continue; break; } j; }), "int i=0; int j=0; for(;!i;) { for (;j!=10;j++) continue; break; } j;");
    assert(11, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } i; }), "int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } i;");
    assert(5, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } j; }), "int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } j;");
    assert(11, ({ int i=0; int j=0; while(!i) { while (j++!=10) continue; break; } j; }), "int i=0; int j=0; while(!i) { while (j++!=10) continue; break; } j;");

    assert(3, ({ int x[2]; x[0]=3; param_decay(x); }), "int x[2]; x[0]=3; param_decay(x);");

    assert(3, ({ int i=0; goto a; a: i++; b: i++; c: i++; i; }), "int i=0; goto a; a: i++; b: i++; c: i++; i;");
    assert(2, ({ int i=0; goto e; d: i++; e: i++; f: i++; i; }), "int i=0; goto d; d: i++; e: i++; f: i++; i;");
    assert(1, ({ int i=0; goto i; g: i++; h: i++; i: i++; i; }), "int i=0; goto g; h: i++; i: i++; j: i++; i;");

    assert(5, ({ int i=0; switch(0) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "int i=0; switch(0) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i;");
    assert(6, ({ int i=0; switch(1) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "int i=0; switch(1) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i;");
    assert(7, ({ int i=0; switch(2) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "int i=0; switch(2) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i;");
    assert(0, ({ int i=0; switch(3) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "int i=0; switch(3) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i;");
    assert(5, ({ int i=0; switch(0) { case 0:i=5;break; default:i=7; } i; }), "int i=0; switch(0) { case 0:i=5;break; default:i=7; } i;");
    assert(7, ({ int i=0; switch(1) { case 0:i=5;break; default:i=7; } i; }), "int i=0; switch(1) { case 0:i=5;break; default:i=7; } i;");
    assert(2, ({ int i=0; switch(1) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "int i=0; switch(1) { case 0: 0; case 1: 0; case 2: 0; i=2; } i;");
    assert(0, ({ int i=0; switch(3) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "int i=0; switch(3) { case 0: 0; case 1: 0; case 2: 0; i=2; } i;");

    assert(99, ({ global_voidfn = 100; void_dec_global_val(); global_voidfn; }), "global_voidfn = 100; void_dec_global_val(); global_voidfn;");

    assert(1, 1<<0, "1<<0");
    assert(8, 1<<3, "1<<3");
    assert(10, 5<<1, "5<<1");
    assert(2, 5>>1, "5>>1");
    assert(-1, -1>>1, "-1>>1");

    assert(1, ({ int i=1; i<<=0; i; }), "int i=1; i<<0;");
    assert(8, ({ int i=1; i<<=3; i; }), "int i=1; i<<3;");
    assert(10, ({ int i=5; i<<=1; i; }), "int i=5; i<<1;");
    assert(2, ({ int i=5; i>>=1; i; }), "int i=5; i>>1;");
    assert(-1, -1, "-1");
    assert(-1, ({ int i=-1; i; }), "int i=-1; i;");
    assert(-1, ({ int i=-1; i>>=1; i; }), "int i=1; i>>1;");

    assert(100, ({ 1?100:200; }), "1?100:200;");
    assert(200, ({ 0?100:200; }), "0?100:200;");

    assert(0, ({ char x[!1]; sizeof(x); }), "char x[!1]; sizeof(x);");
    assert(1, ({ char x[!(10-10)]; sizeof(x); }), "char x[!(10-10)]; sizeof(x);");
    assert(0, ({ int x[~-1]; sizeof(x); }), "int x[~-1]; sizeof(x);");

    assert(8, ({ int x[1+1]; sizeof(x); }), "int x[1+1] sizeof(x);");
    assert(10, ({ enum { ten=1+2+3+4, }; ten; }), "enum { ten=1+2+3+4, }; ten;");
    assert(1, ({ int i=0; switch(3) { case 5-2+0*3: i++; } i; }), "int i=0; switch(3) { case 5-2+0*3: i++; ); i;");

    assert(12, ({ char x[3<<(6/3)]; sizeof(x); }), "char x[3<<(6/3)]; sizeof(x);");
    assert(1, ({ char x[4>>(1+1)]; sizeof(x); }), "char x[4>>(1+1)]; sizeof(x);");

    assert(1, ({ char x[(1+1)==(1*2)]; sizeof(x); }), "char x[(1+1)==(1*2)]; sizeof(x);");
    assert(0, ({ char x[(1+1)!=(1*2)]; sizeof(x); }), "char x[(1+1)!=(1*2)]; sizeof(x);");

    assert(0, ({ char x[(1-1)>=(1*2)]; sizeof(x); }), "char x[(1-1)>=(1*2)]; sizeof(x);");
    assert(1, ({ char x[(1+1)<=(1*2)]; sizeof(x); }), "char x[(1+1)<=(1*2)]; sizeof(x);");

    assert(0, ({ char x[(1+1)<(1*2)]; sizeof(x); }), "char x[(1+1)<(1*2)]; sizeof(x);");
    assert(1, ({ char x[(1+2)>(1*2)]; sizeof(x); }), "char x[(1+2)>(1*2)]; sizeof(x);");

    assert(8, ({ char x[11&56]; sizeof(x); }), "char x[11&56]; sizeof(x);");
    assert(59, ({ char x[11|56]; sizeof(x); }), "char x[11|56]; sizeof(x);");
    assert(51, ({ char x[11^56]; sizeof(x); }), "char x[11^56]; sizeof(x);");

    assert(1, ({ char x[1&&1]; sizeof(x); }), "char x[1&&1]; sizeof(x);");
    assert(0, ({ char x[1&&0]; sizeof(x); }), "char x[1&&0]; sizeof(x);");
    assert(1, ({ char x[0||1]; sizeof(x); }), "char x[0||1]; sizeof(x);");
    assert(0, ({ char x[0||0]; sizeof(x); }), "char x[0||0]; sizeof(x);");

    assert(0, ({ char x[!1]; sizeof(x); }), "char x[!1]; sizeof(x);");
    assert(1, ({ char x[!(10-10)]; sizeof(x); }), "char x[!(10-10)]; sizeof(x);");
    assert(0, ({ int x[~-1]; sizeof(x); }), "int x[~-1]; sizeof(x);");

    assert(1, ({ char x[1?1:2]; sizeof(x); }), "char x[1?1:2]; sizeof(x);");
    assert(2, ({ char x[0?1:2]; sizeof(x); }), "char x[0?1:2]; sizeof(x);");

    assert(3, ({ char x[(1,2,3)]; sizeof(x); }), "char x[(1,2,3)]; sizeof(x);");

    assert(1, ({ int x[3]={1,2,3}; x[0]; }), "int x[3]={1,2,3}; x[0];");
    assert(2, ({ int x[3]={1,2,3}; x[1]; }), "int x[3]={1,2,3}; x[1];");
    assert(3, ({ int x[3]={1,2,3}; x[2]; }), "int x[3]={1,2,3}; x[2];");
    assert(3, ({ int x[3]={1,2,3,}; x[2]; }), "int x[3]={1,2,3,}; x[2];");
    assert(2, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[0][1]; }), "int x[2][3]={{1,2,3},{4,5,6}}; x[0][1];");
    assert(4, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][0]; }), "int x[2][3]={{1,2,3},{4,5,6}}; x[1][0];");
    assert(6, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][2]; }), "int x[2][3]={{1,2,3},{4,5,6}}; x[1][2];");

    assert(0, ({ int x[3]={}; x[0]; }), "int x[3]={}; x[0];");
    assert(0, ({ int x[3]={}; x[1]; }), "int x[3]={}; x[1];");
    assert(0, ({ int x[3]={}; x[2]; }), "int x[3]={}; x[2];");
    assert(2, ({ int x[2][3]={{1,2}}; x[0][1]; }), "int x[2][3]={{1,2}}; x[0][1];");
    assert(0, ({ int x[2][3]={{1,2}}; x[1][0]; }), "int x[2][3]={{1,2}}; x[1][0];");
    assert(0, ({ int x[2][3]={{1,2}}; x[1][2]; }), "int x[2][3]={{1,2}}; x[1][2];");

    assert('c', ({ char x[3] = "c2"; x[0]; }), "char x[3] = \"c2\"; x[0];");
    assert('2', ({ char x[3] = "c2"; x[1]; }), "char x[3] = \"c2\"; x[1];");
    assert(0,   ({ char x[3] = "c2"; x[2]; }), "char x[3] = \"c2\"; x[2];");

    assert('c', ({ char x[2][3] = {"c2", "v0" }; x[0][0]; }), "char x[2][3] = {{\"c2\", \"v0\"}; x[0][0]}; ");
    assert(0, ({ char x[2][3] = {"c2", "v0" }; x[1][2]; }), "char x[2][3] = {{\"c2\", \"v0\"}; x[0][2]}; ");
    assert(0, ({ char x[2][3] = {"c2", "v0" }; x[1][2]; }), "char x[2][3] = {{\"c2\", \"v0\"}; x[1][2]}; ");

    assert(4, ({ int x[]={1,2,3,4}; x[3]; }), "int x[]={1,2,3,4}; x[3];");
    assert(16, ({ int x[]={1,2,3,4}; sizeof(x); }), "int x[]={1,2,3,4}; sizeof(x);");
    assert(4, ({ char x[]="foo"; sizeof(x); }), "char x[]=\"foo\"; sizeof(x); }");
    assert('f', ({ char *x="fox"; x[0]; }), "char *x=\"fox\"; x[0]; }");
    assert('o', ({ char *x="fox"; x[1]; }), "char *x=\"fox\"; x[1]; }");
    assert('x', ({ char *x="fox"; x[2]; }), "char *x=\"fox\"; x[2]; }");

    assert(1, ({ struct {int a; int b; int c;} x={1,2,3}; x.a; }), "struct {int a; int b; int c;} x={1,2,3}; x.a;");
    assert(2, ({ struct {int a; int b; int c;} x={1,2,3}; x.b; }), "struct {int a; int b; int c;} x={1,2,3}; x.b;");
    assert(3, ({ struct {int a; int b; int c;} x={1,2,3}; x.c; }), "struct {int a; int b; int c;} x={1,2,3}; x.c;");
    assert(1, ({ struct {int a; int b; int c;} x={1}; x.a; }), "struct {int a; int b; int c;} x={1}; x.a;");
    assert(0, ({ struct {int a; int b; int c;} x={1}; x.b; }), "struct {int a; int b; int c;} x={1}; x.b;");
    assert(0, ({ struct {int a; int b; int c;} x={1}; x.c; }), "struct {int a; int b; int c;} x={1}; x.c;");

    assert(1, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].a; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].a;");
    assert(2, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].b; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].b;");
    assert(3, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].a; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].a;");
    assert(4, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].b; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].b;");

    assert(0, ({ struct {int a; int b;} x[2]={{1,2}}; x[1].b; }), "struct {int a; int b;} x[2]={{1,2}}; x[1].b;");

    assert(0, ({ struct {int a; int b;} x={}; x.a; }), "struct {int a; int b;} x={}; x.a;");
    assert(0, ({ struct {int a; int b;} x={}; x.b; }), "struct {int a; int b;} x={}; x.b;");

    assert(7, ({ int i=0; int j=0; do { j++; } while (i++ < 6); j; }), "int i=0; int j=0; do { j++; } while (i++ < 6); j;");
    assert(4, ({ int i=0; int j=0; int k=0; do { if (++j > 3) break; continue; k++; } while (1); j; }), "int i=0; int j=0; int k=0; do { if (j++ > 3) break; continue; k++; } while (1); j;");

    assert(2, ({ int i=6; i&=3; i; }), "int i=6; i&=3; i;");
    assert(7, ({ int i=6; i|=3; i; }), "int i=6; i|=3; i;");
    assert(10, ({ int i=15; i^=5; i; }), "int 15; i^=5; i;");
    
    assert(3, g3, "g3");
    assert(4, g4, "g4");
    assert(5, g5, "g5");
    assert(6, g6, "g6");
    assert(5, *g7, "*g7");
    assert(0, strcmp(g8, "abc"), "strcmp(g8, \"abc\")");

    assert(0, g9[0], "g9[0]");
    assert(1, g9[1], "g9[1]");
    assert(0, g9[2], "g9[2]");

    assert(0, strcmp(g10[0], "foo"), "strcmp(g10[0], \"foo\")");
    assert(0, strcmp(g10[1], "bar"), "strcmp(g10[1], \"bar\")");
    assert(0, g10[1][3], "g10[1][3]");
    assert(2, sizeof(g10) / sizeof(*g10), "sizeof(g10) / sizeof(*g10)");

    assert(1, g11[0].a, "g11[0].a");
    assert(2, g11[0].b, "g11[0].b");
    assert(3, g11[1].a, "g11[1].a");
    assert(4, g11[1].b, "g11[1].b");

    assert(1, g12[0].a[0], "g12[0].a[0]");
    assert(2, g12[0].a[1], "g12[0].a[1]");
    assert(0, g12[1].a[0], "g12[1].a[0]");
    assert(0, g12[1].a[1], "g12[1].a[1]");

    assert(1, g13[0].a[0], "g13[0].a[0]");
    assert(2, g13[0].a[1], "g13[0].a[1]");
    assert(3, g13[1].a[0], "g13[1].a[0]");
    assert(4, g13[1].a[1], "g13[1].a[1]");
    
    assert(0, ({ int x[2][3]={0,1,2,3,4,5,}; x[0][0]; }), "int x[2][3]={0,1,2,3,4,5,}; x[0][0];");
    assert(3, ({ int x[2][3]={0,1,2,3,4,5,}; x[1][0]; }), "int x[2][3]={0,1,2,3,4,5,}; x[1][0];");

    assert(0, ({ struct {int a; int b;} x[2]={0,1,2,3}; x[0].a; }), "struct {int a; int b;} x[2]={0,1,2,3}; x[0].a;");
    assert(2, ({ struct {int a; int b;} x[2]={0,1,2,3}; x[1].a; }), "struct {int a; int b;} x[2]={0,1,2,3}; x[1].a;");

    assert(0, strcmp(g15, "foo"), "strcmp(g15, \"foo\")");
    assert(0, strcmp(g16[0], "foo"), "strcmp(g16[0], \"foo\")");
    assert(0, strcmp(g16[1], "bar"), "strcmp(g16[1], \"bar\")");
  
    assert(7, sizeof(g17), "sizeof(g17)");
    assert(10, sizeof(g18), "sizeof(g18)");
    assert(3, sizeof(g19), "sizeof(g19)");

    assert(0, memcmp(g17, "foobar", 7), "memcmp(g17, \"foobar\", 7)");
    assert(0, memcmp(g18, "foobar\0\0\0", 10), "memcmp(g18, \"foobar\\0\\0\\0\", 10)");
    assert(0, memcmp(g19, "foo", 3), "memcmp(g19, \"foo\", 3)");

    assert(0, strcmp(g20, "foobar"), "strcmp(g20, \"foobar\")");
    assert(0, strcmp(g21, "bar"), "strcmp(g21, \"bar\")");
    assert(0, strcmp(g22+3, "foobar"), "strcmp(g22+3, \"foobar\")");

    assert(0, strcmp(g23[0], "foobar"), "strcmp(g23[0], \"foobar\")");
    assert(0, strcmp(g23[1], "bar"), "strcmp(g23[1], \"bar\")");
    assert(0, strcmp(g23[2]+3, "foobar"), "strcmp(g23[2]+3, \"foobar\")");

    assert(3, g24, "g24");
    assert(3, *g25, "*g25");
    assert(2, *g27, "*g27");
    
    ext1 = 5;
    assert(5, ext1, "ext1");

    ext2 = &ext1;
    assert(5, *ext2, "*ext2");

    ;

    assert(1, _Alignof(char), "_Alignof(char)");
    assert(2, _Alignof(short), "_Alignof(short)");
    assert(4, _Alignof(int), "_Alignof(int)");
    assert(8, _Alignof(long), "_Alignof(long)");
    assert(8, _Alignof(long long), "_Alignof(long long)");
    assert(1, _Alignof(char[3]), "_Alignof(char[3])");
    assert(4, _Alignof(int[3]), "_Alignof(int[3])");
    assert(1, _Alignof(struct {char a; char b;}[2]), "_Alignof(struct {char a; char b;}[2])");
    assert(8, _Alignof(struct {char a; long b;}[2]), "_Alignof(struct {char a; long b;}[2])");

    assert(2, counter(), "counter()");
    assert(4, counter(), "counter()");
    assert(6, counter(), "counter()");

    assert(8, ({ struct *foo; sizeof(foo); }), "struct *foo; sizeof(foo);");
    assert(4, ({ struct T *foo; struct T {int x;}; sizeof(struct T); }), "struct T *foo; struct T {int x;}; sizeof(struct T);");
    assert(1, ({ struct T { struct T *next; int x; } a; struct T b; b.x=1; a.next=&b; a.next->x; }), "struct T { struct T *next; int x; } a; struct T b; b.x=1; a.next=&b; a.next->x;");

    assert(1, (int){1}, "(int){1}");
    assert(2, ((int[]){0,1,2})[2], "(int[]){0,1,2}[2]");
    assert('a', ((struct {char a; int b;}){'a', 3}).a, "((struct {char a; int b;}){'a', 3}).a");
    assert(3, ({ int x=3; (int){x}; }), "int x=3; (int){x};");

    assert(1, tree->val, "tree->val");
    assert(2, tree->lhs->val, "tree->lhs->val");
    assert(3, tree->lhs->lhs->val, "tree->lhs->lhs->val");
    assert(4, tree->lhs->rhs->val, "tree->lhs->rhs->val");

    assert(3, ext3, "ext3");

    assert(1, true_fn(), "true_fn()");
    assert(0, false_fn(), "false_fn()");

    printf("OK\n");
    return 0;
}

