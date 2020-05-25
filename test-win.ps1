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