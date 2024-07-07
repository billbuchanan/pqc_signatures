gcc -c *.c -D HASH=sha256 -D THASH=robust
gcc   -o sphincs192.exe *.o -llibcrypto -llibssl