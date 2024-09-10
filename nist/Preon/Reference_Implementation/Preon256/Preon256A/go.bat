gcc -c *.c  -D USE_PREON256A
gcc   -o preon1.exe *.o -llibcrypto -llibssl -lgmp
