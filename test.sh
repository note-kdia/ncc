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

True=1
False=0

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 18 '3 * 6;'
assert 3 '21 / 7;'
assert 15 '3+6*2;'
assert 5 '14-6/2*3;'
assert 23 '6/2*3+2*7;'
assert 18 '3 *(  2+4 );'
assert 5 '70/(2+4*3);'
assert 10 '-10+20;'
assert 15 '10/-2+20;'
assert $True '2>1;'
assert $False '1 >= 5;'
assert $True '211 == 211;'
assert $True '211 == +211;'
assert $True '5 != 0;'
assert $False '6 < -3;'
assert $True '5 > 4 == 1;'
assert $False '-10 >= 35;'
assert $True '-10 <= 35;'
assert 10 '1+2;1+8;5+5;'
assert 8 'a=5;b=3;a+b;'
assert 16 'a=5 ;b=3;-a+b*7;'
assert 9 'foo=7;bar=2;foo+bar;'
assert 10 'var1=7;var2=3;var1+var2;'
assert 11 'var_=7;var=4;var_+var;'
assert 0 'return 0;return 1;'
assert 10 'foo=9; bar=1; return foo+bar;'
assert 1 'if (1==1) return 1; return 0;'
assert 0 'if (1!=1) return 1; return 0;'
assert 1 'if (1==1) if (2==2) return 1; return 0;'
assert 0 'if (1==1) if (2!=2) return 1; return 0;'

echo OK
