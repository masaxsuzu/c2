ls .\win\*.c | % {
    $src = $_.Name
    $obj = $src.Replace(".c", ".obj")
    cl .\win\$src /Fo $obj
}

link /OUT:c2-gen1-win.exe .\codegen.obj .\lib.obj .\main.obj .\parse.obj .\tokenize.obj .\type.obj