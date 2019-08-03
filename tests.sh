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

echo OK
