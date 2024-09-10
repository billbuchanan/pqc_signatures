cp .\api\api.h.1.fe api.h
gcc -c aes\*.c 
gcc -c keccak\*.c
gcc -c *.c 
gcc   -o alteq1.exe *.o -llibcrypto -llibssl

cp .\api\api.h.3.fe api.h
gcc -c aes\*.c 
gcc -c keccak\*.c
gcc -c *.c 
gcc   -o alteq3.exe *.o -llibcrypto -llibssl

cp .\api\api.h.5.fe api.h
gcc -c aes\*.c 
gcc -c keccak\*.c
gcc -c *.c 
gcc   -o alteq5.exe *.o -llibcrypto -llibssl