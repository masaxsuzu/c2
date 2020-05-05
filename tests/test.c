int assert(int want, int got, char *code) {
    if (want != got) {
        printf("%s => %d expected but got %d\n", code, want, got);
        exit(1);
    }
    printf("%s => %d\n", code, got);
    return 0;
}

int ret3() {
    return 3;
}
int ret42(int x, int y) {
    return x*y;
}
int fib(int n) {
    if (n <= 1) {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}

int fninfn(int x) { 
    return x; 
} 
int nested(int x, int y) { 
    if (x==y) 
        return fninfn(x); 
    return 1; 
}

// return -1; line comment

/*
    return 255; block comment
*/

int main() {
    assert(0, 0, "0");
    assert(42, 42, "42");

    assert(21, 5 + 20 - 4, "5+20-4");
    assert(41, 12 + 34 - 5, "12 + 34 - 5");
    assert(8, 4 * 2, "4*2");
    assert(33, 99 / 3, "99/3");
    assert(128, (128), "(128)");
    assert(6, (1 + 2) * 4 / 2, "(1+2) * 4 / 2");
    assert(100, +100, "+100");
    assert(100, -100 + 200, "-100+200");

    assert(0, 0 == 1, "0==1");
    assert(1, 42 == 42, "42==42");
    assert(1, 0 != 1, "0!=1");
    assert(0, 42 != 42, "42!=42");

    assert(1, 0 < 1, "0 < 1");
    assert(0, 1 < 1, "1 < 1");
    assert(0, 2 < 1, "2 < 1");
    assert(1, 0 <= 1, "0 <= 1");
    assert(1, 1 <= 1, "1 <= 1");
    assert(0, 2 <= 1, "2 <= 1");

    assert(0, 0 > 1, "0 > 1");
    assert(0, 1 > 1, "1 > 1");
    assert(1, 2 > 1, "2 > 1");
    assert(0, 0 >= 1, "0 >= 1");
    assert(1, 1 >= 1, "1 >= 1");
    assert(1, 2 >= 1, "2 >= 1");

    assert(3, ret3(), "ret3()");
    assert(42, ret42(7,6), "ret42(7,6)");
    assert(55, fib(9), "fib(9)");

    assert(1, nested(1,100), "nested(1,100)");
    assert(100, nested(100,100), "nested(100,100)");

    assert(97, "abc"[0], "\"abc\"[0]");
    assert(98, "abc"[1], "\"abc\"[1]");
    assert(99, "abc"[2], "\"abc\"[2]");

    assert(7,"\a"[0], "\"\\a\"[0]\"");
    assert(8,"\b"[0], "\"\\b\"[0]\"");
    assert(9,"\t"[0], "\"\\t\"[0]\"");
    assert(10,"\n"[0], "\"\\n\"[0]\"");
    assert(11,"\v"[0], "\"\\v\"[0]\"");
    assert(12,"\f"[0], "\"\\f\"[0]\"");
    assert(13,"\r"[0], "\"\\r\"[0]\"");
    assert(27,"\e"[0], "\"\\e\"[0]\"");
    assert(0,"\0"[0], "\"\\0\"[0]\"");
    assert(106,"\j"[0], "\"\\j\"[0]\"");
    assert(107,"\k"[0], "\"\\k\"[0]\"");
    assert(108,"\l"[0], "\"\\l\"[0]\"");

    assert(123, ({ 123; }), "({ 123;})");
    assert(3, ({ 1;2;3; }), "({ 1;2;3;})");
    assert(255, ({ int x = 255; x;}), "({ int x = 255; x;})");

    printf("OK\n");
    return 0;
}
