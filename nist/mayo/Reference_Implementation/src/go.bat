gcc -c mayo_1\*.c -I ..\include -D ENABLE_PARAMS_DYNAMIC
gcc -c common\*.c 
gcc -c *.c -I ..\include -I common -I generic -I ..\src -D ENABLE_PARAMS_DYNAMIC
gcc   -o mayo_1.exe *.o  -llibcrypto -llibssl