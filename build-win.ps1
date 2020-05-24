ls .\src\*.c | % {
    $src = $_.Name
    $obj = $src.Replace(".c", ".obj")
    gcc .\src\$src -std=c11 -g -static -fno-common -v -c -o .\win\$obj
}

ls .\patch\*.c | % {
    $src = $_.Name
    $obj = $src.Replace(".c", ".obj")
    gcc .\patch\$src -std=c11 -g -static -fno-common -v -c -o .\win\$obj
}

gcc -o c2-gen1-win .\win\codegenwin.obj .\win\lib.obj .\win\main.obj .\win\parse.obj .\win\tokenize.obj .\win\type.obj -std=c11 -g -static -fno-common