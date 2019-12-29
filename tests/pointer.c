int main() {
    int a=2;
    int *b = &a;
    a=100;
    int c = *b;
    return c;
}