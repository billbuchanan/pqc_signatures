gcc -c *.c -D HASH=sha256 -D THASH=robust
gcc   -o sphincs128.exe *.o -llibcrypto -llibssl