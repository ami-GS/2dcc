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
try 3 "return 1+2;"
try 2 "if (1 == 1) 1+1;"
try 4 "if (1 == 0) 1+1; else 2+2;"
try 3 "int i = 0;
while (i < 3) i = i + 1;"
try 3 "{ int a=1; int c = 4; {int b=2; a+b;} }"
try 4 "int abc=4;"
try 3 "int abc=5;
int bcd=3;"
try 9 "int abc=5;
int bcd=4;
abc+bcd;"
try 10 "int a=2;
int b = &a;
a=10;

echo OK
