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

echo OK
