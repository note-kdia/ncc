#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./ncc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42
assert 21 '5+20-4'
assert 41 ' 12 + 34 - 5 '
assert 18 '3 * 6'
assert 3 '21 / 7'
assert 15 '3+6*2'
assert 5 '14-6/2*3'
assert 23 '6/2*3+2*7'
assert 18 '3 *(  2+4 )'
assert 5 '70/(2+4*3)'

echo OK
