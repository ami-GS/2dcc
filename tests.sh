#!/bin/bash

for file in ./tests/*.c; do
    ./2dcc ${file} > tmp.s
    if [[ "$file" == *"global"* ]]; then
        # need to remove static
        gcc -static -g -O0 -o tmp tmp.s
    else
        gcc -g -O0 -o tmp tmp.s
    fi
    ./tmp
    actual=$?
    if [ ${actual} == 100 ]; then
        echo -e "${file}:\t SUCCESS => 100"
    else
        echo -e "${file}:\t FAILED => ${actual}"
        exit 1
    fi
done