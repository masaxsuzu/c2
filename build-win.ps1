ls .\src\*.c | % {
    $src = $_.Name
    $obj = $src.Replace(".c", ".obj")
    cl .\src\$src /Fo $obj
}

ls .\patch\*.c | % {
    $src = $_.Name
    $obj = $src.Replace(".c", ".obj")
    cl .\patch\$src /Fo $obj
}

link /OUT:c2-gen1-win.exe .\codegenwin.obj .\libwin.obj .\main.obj .\parse.obj .\tokenize.obj .\type.obj