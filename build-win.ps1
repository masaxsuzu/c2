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

link /OUT:c2-gen1-win.exe .\codegenwin.obj .\lib.obj .\mainwin.obj .\parse.obj .\tokenize.obj .\type.obj