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
Assert 128 'int main() { return 1 + (6/3)*64 -1;}'
Assert 129 'int main() { int x = 2; return x + (6/3)*64 -1;}'
Assert 129 'int main() { int x = 2; int y = 3; return x + (6/y)*64 -1;}'
Assert 4 'int main() { int x1 = 1; int x2 = 2; int x3 = 3; int x4 = 4; int x5 = 5; int x6 = 6; int x7 = 7; return (x1 + x7) / x2;}'
Assert 1 'void f() {} int main() { return 1;}'

Write-Host("OK")