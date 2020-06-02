function Assert {
    param (
        $want,
        $A,
        $B
    )
    .\tools\peparser_x64.exe --compare .\$A .\$B 
    $got = $lastexitcode
    if ($want -ne $got) {
        exit 1
    }
}

Assert 1 c2-gen2-win.exe c2-gen1-win.exe
Assert 0 c2-gen2-win.exe c2-gen3-win.exe
