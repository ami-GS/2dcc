int main() {
    int a = 90;
    int b;
    int c = 20;
    int d = 0;
    b = ++c + a++ - 11 + d--; // b==98
    return b;
}