cmd /c ".\c2-gen1-win.exe .\tests\win.c > .\win.asm"
ml64 .\win.asm
link /OUT:.\tmp.exe .\win.obj
.\tmp.exe

If ($LASTEXITCODE -ne "128") {
    Write-Error("Fail")
    exit -1
}

Write-Host("OK")