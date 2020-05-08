int assert(int want, int got, char *code) {
    if (want != got) {
        printf("%s => %d expected but got %d\n", code, want, got);
        exit(1);
    }
    printf("%s => %d\n", code, got);
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

    printf("OK\n");
    return 0;
}
