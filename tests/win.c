void assert(int got, int want, char *src) { 
    if (got != want) {
        printf("%s => %d, want %d\n", src, got, want); 
        return exit(1);
    }
    printf("%s => %d\n", src, got); 
} 

_Bool ok(_Bool x, _Bool y) {
    return x == y;
}

int main() { 
    
    assert(1, ({ ok(0,0); }), "ok(0,0);");
    assert(0, ({ ok(1,0); }), "ok(1,0);");
    assert(1, ({ (_Bool)10; }), "(_Bool)10;");
    assert(1, ({ sizeof(_Bool); }), "sizeof(_Bool);");

    assert(42, 42, "42");

    printf("OK\n");
    return 0;
}