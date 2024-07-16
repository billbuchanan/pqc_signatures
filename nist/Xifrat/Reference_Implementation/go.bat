gcc -c *.c  -D USE_PREON128A
gcc   -o Xifrat1.exe *.o -llibcrypto -llibssl -lgmp
