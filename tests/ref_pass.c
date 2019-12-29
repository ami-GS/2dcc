void func(int b, int *a) {
    *a = *a + b;
}
int main() {
    int a = 87;
    func(13, &a);
    return a;
}