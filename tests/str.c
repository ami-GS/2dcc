char func(char* b) {
    return b[3];
}
int main() {
    char b[5] = "abcde";
    return func(b);
}