$TMP='tmp-self'
$genA=$args[0]
$genB=$args[1];
$Utf8NoBomEncoding = New-Object System.Text.UTF8Encoding $False

function expand {
    param (
        $file
    )
    $def = ' 
int exit();
int fprintf();
int vprintf();
int vfprintf();
int ispunct();
int isdigit();
int memcmp();
int strdup();
typedef struct FILE FILE;
FILE *stdout;
FILE *stderr;
void *malloc(long size);
void *calloc(long nmemb, long size);
int *_errno();
char *strerror(int errnum);
FILE *fopen(char *pathname, char *mode);
long fread(void *ptr, long size, long nmemb, FILE *stream);
int feof(FILE *stream);
int strcmp(char *s1, char *s2);
int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
long strlen(char *p);
int strncmp(char *p, char *q);
void *memcpy(char *dst, char *src, long n);
int isspace(int c);
char *strstr(char *haystack, char *needle);                                              
long strtol(char *nptr, char **endptr, int base);
typedef struct {
int gp_offset;
int fp_offset;
void *overflow_arg_area;
void *reg_save_area;
} __va_elem;
typedef __va_elem va_list[1];
static void va_start(__va_elem *ap) {
__builtin_va_start(ap);
}
static void va_end(__va_elem *ap) {}'

    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $def, $Utf8NoBomEncoding)

    $header = $(Get-Content .\win\c2.h | ? {$_ -NotMatch "^#"})
    $src = $(Get-Content .\win\$file | ? {$_ -NotMatch "^#"})

    [System.IO.File]::AppendAllLines([string]".\$TMP\${file}", [string[]]$header)
    [System.IO.File]::AppendAllLines([string]".\$TMP\${file}", [string[]]$src)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -creplace "\bbool\b","_Bool" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -creplace "\berrno\b","*_errno()" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -creplace "\btrue\b","1" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -creplace "\bfalse\b","0" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -creplace "\bNULL\b","0" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $expanded = $(Get-Content .\$TMP\${file} | % { $_ -replace ", \.\.\.","" } )
    [System.IO.File]::WriteAllLines(".\$TMP\${file}", $expanded, $Utf8NoBomEncoding)

    $asm = invoke-expression ".\${genA} .\$TMP\${file}"
    $out = $file.Replace(".c", ".asm")
    $obj = $file.Replace(".c", ".obj")

    [System.IO.File]::WriteAllLines(".\${out}", $asm, $Utf8NoBomEncoding)
    ml64 $out /c
}

rm *.asm
rm *.obj

mkdir -f $TMP

rm main.*
rm lib.*
rm type.*
rm tokenize.*
rm codegen.*
rm parse.*

// As of now, I need this dummy object.
cl /TC /Fo /Fa .\win\msvc.c 

expand main.c
expand lib.c
expand type.c
expand tokenize.c
expand codegen.c
expand parse.c

link /OUT:$genB .\codegen.obj .\lib.obj .\main.obj .\parse.obj .\tokenize.obj .\type.obj .\msvc.obj legacy_stdio_definitions.lib /FORCE

