function Assert {
    param (
        $want,
        $src
    )

    cmd /c "echo $src > .\tmp.c"
    cmd /c ".\c2-gen1-win.exe .\tmp.c > .\win.asm"
    ml64 .\win.asm
    link /OUT:.\tmp.exe .\win.obj
    .\tmp.exe
    $got = $LASTEXITCODE
    If ($got -ne "$want") {
        Write-Error("Fail: ${src} => ${got}, want ${want}")
        exit -1
    }
    Write-Output("${src} => ${got}")
}

# Global variable
Assert 100 'int x; int main() { x = 100; return x;}'

# Comma
Assert 55 'int main() { return (1,2,55);}'

# And or
Assert 0 'int main() { return 0^&^&0;}'
Assert 0 'int main() { return 0^&^&1;}'
Assert 0 'int main() { return 1^&^&0;}'
Assert 1 'int main() { return 1^&^&1;}'

Assert 0 'int main() { return 0^|^|0;}'
Assert 1 'int main() { return 0^|^|1;}'
Assert 1 'int main() { return 1^|^|0;}'
Assert 1 'int main() { return 1^|^|1;}'

# cast 
Assert 0 'int main() { return (short)65536;}'
Assert 0 'int main() { return (char)256;}'

# equals 
Assert 2 'int main() { int x = 1; x+=1; return x;}'
Assert 0 'int main() { int x = 1; x-=1; return x;}'
Assert 6 'int main() { int x = 3; x*=2; return x;}'
Assert 3 'int main() { int x = 12; x/=4; return x;}'
Assert 8 'int main() { int x = 2; x^<^<=2; return x;}'
Assert 8 'int main() { int x = 64; x^>^>=3; return x;}'

# <<
Assert 16 'int main() { return 4^<^<2;}'
Assert 1 'int main() { return 4^>^>2;}'


# !,~,^,|,&
Assert 1 'int main() { return ^!0;}'
Assert 0 'int main() { return ^!1;}'
Assert 2 'int main() { return ~-3;}'
Assert 4 'int main() { return ~-5;}'
Assert 0 'int main() { return 0^^0;}'
Assert 1 'int main() { return 0^^1;}'
Assert 1 'int main() { return 1^^0;}'
Assert 0 'int main() { return 1^^1;}'
Assert 0 'int main() { return 0^|0;}'
Assert 1 'int main() { return 0^|1;}'
Assert 1 'int main() { return 1^|0;}'
Assert 1 'int main() { return 1^|1;}'
Assert 0 'int main() { return 0^&0;}'
Assert 0 'int main() { return 0^&1;}'
Assert 0 'int main() { return 1^&0;}'
Assert 1 'int main() { return 1^&1;}'


# ?:
Assert 64 'int main() { return 1?64:100;}'
Assert 100 'int main() { return 0?64:100;}'

# ++/--
Assert 1 'int main() { int x = 1; return x++;}'
Assert 1 'int main() { int x = 1; return x--;}'
Assert 2 'int main() { int x = 1; return ++x;}'
Assert 0 'int main() { int x = 1; return --x;}'

# lt/le eq
Assert 8 'int main() { if(0^<1) {return 8;} return 100;}'
Assert 100 'int main() { if(0^<0) {return 8;} return 100;}'
Assert 8 'int main() { if(1^<=2) {return 8;} else { return 100;} return 19;}'
Assert 100 'int main() { if(2^<=1) {return 8;} else { return 100;} return 19;}'

# eq/not eq
Assert 8 'int main() { if(0==0) {return 8;} return 100;}'
Assert 100 'int main() { if(1==0) {return 8;} return 100;}'
Assert 8 'int main() { if(1!=0) {return 8;} else { return 100;} return 19;}'
Assert 100 'int main() { if(1!=1) {return 8;} else { return 100;} return 19;}'

# if-else
Assert 8 'int main() { if(1) {return 8;} return 100;}'
Assert 100 'int main() { if(0) {return 8;} return 100;}'
Assert 8 'int main() { if(1) {return 8;} else { return 100;} return 19;}'
Assert 100 'int main() { if(0) {return 8;} else { return 100;} return 19;}'

# array of x (char, short, int)
Assert 8 'int main() { char x[8]; return sizeof(x);}'
Assert 16 'int main() { short x[8]; return sizeof(x);}'
Assert 32 'int main() { int x[8]; return sizeof(x);}'
Assert 8 'int main() { int x[4]; x[0] = 8; return x[0];}'
Assert 8 'int main() { int x[4]; x[3] = 8; return x[3];}'
Assert 0 'int main() { int x[2][3]; int *y; y=x; y[0]=0; return x[0][0];}'
Assert 1 'int main() { int x[2][3]; int *y; y=x; y[1]=1; return x[0][1];}'
Assert 2 'int main() { int x[2][3]; int *y; y=x; y[2]=2; return x[0][2];}'
Assert 3 'int main() { int x[2][3]; int *y; y=x; y[3]=3; return x[1][0];}'
Assert 4 'int main() { int x[2][3]; int *y; y=x; y[4]=4; return x[1][1];}'
Assert 5 'int main() { int x[2][3]; int *y; y=x; y[5]=5; return x[1][2];}'
Assert 99 'int main() { char x[4]; x[0] = ''c''; return x[0];}'

# pointer to x(char, short, int)
Assert 8 'int main() { short *x; return sizeof(x);}'
Assert 8 'int main() { char *x; return sizeof(x);}'
Assert 8 'int main() { int *x; return sizeof(x);}'
Assert 5 'int main() { int x; int *y; x=3; y=^&x; *y=5; return x;}' # ^& is treated as & in cmd
Assert 7 'int main() { int x; int y;x=3; y=5; *(^&x+1)=7; return y;}' # ^& is treated as & in cmd
Assert 7 'int main() { int x; int y;x=3; y=5; *(^&y-1)=7; return x;}' # ^& is treated as & in cmd
Assert 0 'int main() {int x; x=3; return ^&x-^&x;}' # ^& is treated as & in cmd
Assert 2 'int main() {int x; x=3; return ^&x+2-^&x;}' # ^& is treated as & in cmd

# short
Assert 2 'int main() { short x = 1; return sizeof(x);}'
Assert 2 'int main() { return sizeof(short);}'
Assert 129 'int main() { short x = 2; return x + (6/3)*64 -1;}'
Assert 129 'int main() { short x = 2; short y = 3; return x + (6/y)*64 -1;}'
Assert 4 'int main() { short x1 = 1; short x2 = 2; short x3 = 3; short x4 = 4; short x5 = 5; short x6 = 6; short x7 = 7; return (x1 + x7) / x2;}'
Assert 3 'short one() { return 1;} int main() { return 2 + one();}'
Assert 3 'short one() { return 1;} short two() { short x = 1; return 2*x;} int main() { return two() + one();}'
Assert 3 'short zero(int x) {return 0;} short one() { return 1;} short two() { int x = 1; return 2*x;} int main() { return two() + one();}'
Assert 42 'short twice(short x) { return 2*x;} int main() { return twice(21);}'
Assert 10 'short sum(short x, short y, short z, short w) { return x+y+z+w; } int main() { return sum(1,2,3,4);}'
Assert 4 'short twice_of_second(short x, short y, short z, short w) { return y*2; } int main() { return twice_of_second(1,2,30,40);}'

# char
Assert 1 'int main() { char x = 1; return sizeof(x);}'
Assert 1 'int main() { return sizeof(char);}'
Assert 129 'int main() { char x = 2; return x + (6/3)*64 -1;}'
Assert 129 'int main() { char x = 2; char y = 3; return x + (6/y)*64 -1;}'
Assert 4 'int main() { char x1 = 1; char x2 = 2; char x3 = 3; char x4 = 4; char x5 = 5; char x6 = 6; char x7 = 7; return (x1 + x7) / x2;}'
Assert 3 'char one() { return 1;} int main() { return 2 + one();}'
Assert 3 'char one() { return 1;} char two() { char x = 1; return 2*x;} int main() { return two() + one();}'
Assert 3 'char zero(int x) {return 0;} char one() { return 1;} char two() { int x = 1; return 2*x;} int main() { return two() + one();}'
Assert 42 'char twice(char x) { return 2*x;} int main() { return twice(21);}'
Assert 10 'char sum(char x, char y, char z, char w) { return x+y+z+w; } int main() { return sum(1,2,3,4);}'
Assert 4 'char twice_of_second(char x, char y, char z, char w) { return y*2; } int main() { return twice_of_second(1,2,30,40);}'
Assert 97 'char a() { return ''a'';} int main() { return a();}'

# int
Assert 42 'int main() { return 42;}'
Assert 4 'int main() { int x = 1; return sizeof(x);}'
Assert 4 'int main() { return sizeof(int);}'
Assert 128 'int main() { return 1 + (6/3)*64 -1;}'
Assert 129 'int main() { int x = 2; return x + (6/3)*64 -1;}'
Assert 129 'int main() { int x = 2; int y = 3; return x + (6/y)*64 -1;}'
Assert 4 'int main() { int x1 = 1; int x2 = 2; int x3 = 3; int x4 = 4; int x5 = 5; int x6 = 6; int x7 = 7; return (x1 + x7) / x2;}'
Assert 1 'void f() {} int main() { return 1;}'
Assert 3 'int one() { return 1;} int main() { return 2 + one();}'
Assert 3 'int one() { return 1;} int two() { int x = 1; return 2*x;} int main() { return two() + one();}'
Assert 3 'int zero(int x) {return 0;} int one() { return 1;} int two() { int x = 1; return 2*x;} int main() { return two() + one();}'
Assert 42 'int twice(int x) { return 2*x;} int main() { return twice(21);}'
Assert 10 'int sum(int x, int y, int z, int w) { return x+y+z+w; } int main() { return sum(1,2,3,4);}'
Assert 4 'int twice_of_second(int x, int y, int z, int w) { return y*2; } int main() { return twice_of_second(1,2,30,40);}'

Write-Host("OK")