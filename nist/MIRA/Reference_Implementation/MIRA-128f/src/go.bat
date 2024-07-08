gcc -c *.c  -I..\lib\randombytes -I..\lib\XKCP -I..\src\wrapper -I..\lib\XKCP\opt64 -I..\src\finite_fields
gcc   -o raccoon1.exe *.o -llibcrypto -llibssl