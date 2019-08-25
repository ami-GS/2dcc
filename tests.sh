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

try 0 0
try 123 123
try 21 "5+20-4"
try 21 " 5 + 20 - 4    "
try 42 "6*7"
try 47 "5+6*7"
try 15 "5*(9-6)"
try 4 "(3+5)/2"
try 5 "3+5/2"


echo OK
