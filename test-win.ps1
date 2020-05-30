function Assert {
    param (
        $want,
        $src
    )

    cmd /c "echo extern printf:proc > .\win.asm" 
    cmd /c "echo extern exit:proc >> .\win.asm" 
    cmd /c "echo extern only_decl:proc >> .\win.asm" 
    cmd /c "echo extern sub_from_last:proc >> .\win.asm" 
    cmd /c ".\c2-gen1-win.exe $src >> .\win.asm"
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

Assert 0 .\tests\win.c

Write-Host("OK")