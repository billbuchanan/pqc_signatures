#!/bin/sh

cd "$(dirname "$0")"

bin=xifrat-funcs-test
src="\
xifrat-funcs-test.c
xifrat-funcs.c
"

wflags="-Wall -Wextra"

clang $wflags -O -o ./$bin $src &&
    dd if=/dev/urandom bs=8 | ./$bin
