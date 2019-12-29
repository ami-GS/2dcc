int fib_recursive(int num) {
    if (num < 0) {
        return 0;
    }
    if (num == 0) {
        return 1;
    }
    if (num == 1) {
        return 1;
    }
    return fib_recursive(num-1) + fib_recursive(num-2);
}

int fib_iterative(int num) {
    int a = 1;
    int b = 1;
    int c = 0;
    for (int i = 1; i < num; i = i + 1) {
        c = b;
        b = a + b;
        a = c;
    }
    return b;
}

int main() {
    if (fib_recursive(10) == fib_iterative(10)) {
        return 100;
    }
    return -1;
}