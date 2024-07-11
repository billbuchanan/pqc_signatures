gcc -c SHAKE\*.c
gcc -c SHAKE\SHA3\*.c
gcc -c *.c 
gcc   -o prov2.exe *.o -llibcrypto -llibssl