void assert(int got, int want, char *src) { 
    if (got != want) {
        printf("%s => %d, want %d\n", src, got, want); 
        return exit(1);
    }
} 

int main() { 
    
    assert(42, 42, "42");

    printf("OK\n");
    return 0;
}