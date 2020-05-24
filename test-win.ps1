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

Write-Host("OK")