gcc -c fips202\*.c
gcc -c *.c -D PARAM_SET_VOX128
gcc   -o vox1.exe *.o -llibcrypto -llibssl
gcc -c *.c -D PARAM_SET_VOX192
gcc   -o vox3.exe *.o -llibcrypto -llibssl
gcc -c *.c -D PARAM_SET_VOX256
gcc   -o vox5.exe *.o -llibcrypto -llibssl