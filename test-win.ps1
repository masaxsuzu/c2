.\win\tmp.exe
If ($LASTEXITCODE -ne "42") {
    Write-Error("Fail")
    exit -1
}

Write-Host("OK")