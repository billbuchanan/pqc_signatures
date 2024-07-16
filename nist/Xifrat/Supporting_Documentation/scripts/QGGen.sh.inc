#!/bin/sh

cd "$(dirname "$0")"
rm -f ./QGGen
main(){ date ; time ./QGGen 16 ; date ; }

if
    cc -o ./QGGen -Wall -Wextra -O3 $src \
       ../../Reference_Implementation/endian.c \
       ../../Reference_Implementation/keccak-f-1600.c \
       ../../Reference_Implementation/shake.c \
       ../../Reference_Implementation/sponge.c
then
    result="$(main "$@" | tee ${src%.c}-result."$*".txt)"
    echo "$result"
fi
