#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./2dcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual"
	exit 1
    fi
}

try 100 "
int main() {
    return 100;
}"
try 100 "
int main() {
    return 12 +88;
}"
try 100 "
int main() {
    return 112 -12 ;
}"
try 100 "
int main() {
    return 25*4;
}"
try 100 "
int main() {
    return 400/4;
}"
try 100 "
int main() {
    return 27*4 + -(32/4);
}"
try 100 "
int main() {
    int a = 32;
    if (a == 32)
        return 100;
}"
try 100 "
int main() {
    int a = 32;
    if (a == 31)
        return 99;
    else
        return 100;
}"
try 234 "
int main() {
    int ret = 1;
    if (ret < 2)
        ret = ret * 2;
    if (ret <= 2)
        ret = 3 * ret;
    if (ret == 6)
        ret = ret * 5;
    if (ret != 29)
        ret = 7 * ret;
    if (ret > 209)
        ret = ret + 11;
    if (ret >= 221)
        ret = 13 + ret;
    return ret;
}"
try 10 "
int main() {
    int a=2;
    int *b = &a;
    a=10;
    int c = *b;
    return c;
}"
try 100 "
int main() {
    int c = 25;
    int ret = 0;
    int i = 0;
    while (i < 4) {
        ret = ret + c;
        i = i + 1;
    }
    return ret;
}"
try 100 "
int main() {
    int c = 25;
    int ret = 0;
    for (int i = 0; i < 4; i = i + 1) {
        ret = ret + c;
    }
    return ret;
}"
try 100 "
int main() {
    // return 101;
    /*
    return 102;
    */
    return 100;
}
"
try 100 "
int func() {
    return 12+88;
}
int main() {
    return func();
}"

try 100 "
int func(int a, int b) {
    return a+b+48;
}
int main() {
    return func(12, 40);
}"

try 100 "
int func(int a, int b) {
    int c = 20;
    int d = 28;
    return a+b+c+d;
}
int main() {
    return func(12, 40);
}"

try 89 "
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
int main() {
    return fib_recursive(10);
}"

try 89 "
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
    return fib_iterative(10);
}"

try 100 "
int main() {
    int a[10];
    int c = 2;
    int b = 30;
    a[c+1] = 70;
    return a[c+1]+ b;
}"

try 100 "
void func(int b, int *a) {
    *a = *a + b;
}
int main() {
    int a = 87;
    func(13, &a);
    return a;
}"

try 100 "
int main() {
    int x;
    int *y;
    y = &x;
    *y = 100;
    return x;
}"

try 100 "
int main() {
    int *a;
    int b = 100;
    a = &b;
    return *a;
}"

try 100 "
int main() {
    int b = 4;
    int a[6] = {92,93,94,95,96,97};

    return a[b] + b;
}"

try 100 "
int main() {
    int a[6][6][6];
    int b = 1;
    int c = 2;
    int d = 3;

    a[b+3][c+2][d+1]= 94 + b + c + d;

    return a[b+3][c+2][d+1];
}"

try 100 "
int pppp(int *array, int C) {
    int ppppArray[5] = {11,22,33,44,55};
    return array[2] + 2*ppppArray[3] + C;
}
int main() {
    int array[3] = {1,2,3};
    return pppp(array, 9);
}"

try 6 "
int main() {
    int a[3] = {1,2,3};
    return sizeof(a) / sizeof(a[0]) + sizeof a / sizeof a[0];
}"
try 100 "
int a = 36;
int b = 37;
int main() {
    return b + 63;
}"
try 100 "
char func(char* b) {
    return b[3];
}
int main() {
    char b[5] = \"abcde\";
    return func(b);
}"

echo OK
