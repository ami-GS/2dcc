int main() {
    int a[6][6][6];
    int b = 1;
    int c = 2;
    int d = 3;

    a[b+3][c+2][d+1]= 94 + b + c + d;

    return a[b+3][c+2][d+1];
}