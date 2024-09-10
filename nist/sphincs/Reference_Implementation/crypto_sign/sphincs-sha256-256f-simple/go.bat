gcc -c *.c -D HASH=sha256 -D THASH=robust
gcc   -o sphincs256.exe *.o -llibcrypto -llibssl