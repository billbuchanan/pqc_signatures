gcc -c *.c -I..\include -D HAETAE_MODE=2
gcc   -o haetae2.exe *.o  -llibcrypto -llibssl
gcc -c *.c -I..\include -D HAETAE_MODE=3
gcc   -o haetae3.exe *.o  -llibcrypto -llibssl
gcc -c *.c -I..\include -D HAETAE_MODE=5
gcc   -o haetae5.exe *.o  -llibcrypto -llibssl