gcc -c *.c  -D USE_PREON192A
gcc   -o preon1.exe *.o -llibcrypto -llibssl -lgmp
