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


try 0 "0;"
try 123 "123;"
try 21 "5+20-4;"
try 21 " 5 + 20 - 4    ;"
try 42 "6*7;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "3+5/2;"
try 2 "-3+5;"
try 8 "+3+5;"
try 5 "-3*5 + 20;"
try 1 "5 == 5 ;"
try 0 " 5== 3;"
try 1 "5 !=3 ;"
try 0 " 5 != 5 ;"
try 0 " 5 < 3 ;"
try 1 " 3 < 5 ;"
try 0 " 3 > 5 ;"
try 1 " 5 > 3 ;"
try 1 " 3 <= 5 ;"
try 1 " 5 <= 5 ;"
try 0 " 5 <= 3 ;"
try 1 " 5 >= 3 ;"
try 1 " 5 >= 5 ;"
try 0 " 3 >= 5 ;"
try 4 "a=4;"
try 3 " a=5;
b=3;"
try 3 "return 1+2;"
try 2 "if (1 == 1) 1+1;"
try 4 "if (1 == 0) 1+1; else 2+2;"
try 9 "a=5;
b=4;
a+b;"
try 6 "ans = 3;
for (i = 0; i < 3; i = i + 1) ans = ans + 1;"
try 3 "i = 0;
while (i < 3) i = i + 1;"
try 3 "{ a=1; c = 4; {b=2; a+b;} }"
try 10 "a=2;
b = &a;
a=10;
c = *b;"



echo OK
