$c2 = $args[0]

function Assert {
    param (
        $want,
        $src
    )

    $Utf8NoBomEncoding = New-Object System.Text.UTF8Encoding $False

    $externs = '
extern printf:proc
extern exit:proc
extern strcmp:proc
extern memcmp:proc'

    $asm = invoke-expression "$c2 $src"
    [System.IO.File]::WriteAllLines(".\win.asm", $externs, $Utf8NoBomEncoding)
    [System.IO.File]::AppendAllLines([string]".\win.asm", [string[]]$asm)
    ml64 .\win.asm
    cl /TC /Fa /Fo .\tests\extern.c
    link /OUT:.\tmp.exe .\win.obj .\extern.obj
    .\tmp.exe
    $got = $LASTEXITCODE
    If ($got -ne "$want") {
        Write-Error("Fail: ${src} => ${got}, want ${want}")
        exit -1
    }
    Write-Output("${src} => ${got}")
}

# Tests in c
# Assert 1 .\tests\debug.c

Assert 0 .\tests\win.c

Write-Host("OK")