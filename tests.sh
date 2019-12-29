#!/bin/bash

assemble() {
    if [[ "$1" == *"global"* ]]; then
        # need to remove static
        gcc -static -g -O0 -o tmp tmp.s
    else
        gcc -g -O0 -o tmp tmp.s
    fi
}

validate() {
    file=$1
    actual=$2
    expected=$3
    if [ ${actual} == ${expected} ]; then
        echo -e "${file}:\t SUCCESS => ${expected}"
    else
        echo -e "${file}:\t FAILED => ${actual}"
        exit 1
    fi
}

if [[ $# == 0 ]]; then
    for file in ./tests/*.c; do
        ./2dcc ${file} > tmp.s
        assemble $file
        ./tmp
        validate $file $? 100
    done
else
    for file in $* ; do
        ./2dcc ${file} > tmp.s
        assemble ${file}
        ./tmp
        validate ${file} $? 100
    done
fi