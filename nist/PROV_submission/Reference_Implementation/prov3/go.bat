gcc -c SHAKE\*.c
gcc -c SHAKE\SHA3\*.c
gcc -c *.c 
gcc   -o prov3.exe *.o -llibcrypto -llibssl