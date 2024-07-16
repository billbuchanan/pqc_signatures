gcc -c *.c  -D USE_PREON128A
gcc   -o preon1.exe *.o -llibcrypto -llibssl -lgmp
