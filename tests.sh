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
    int b = &a;
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
int func() {
    return 12+88;
}
int main() {
    return func();
}"
echo OK
