gcc -c *.c -I..\include -D CATEGORY_1 -D PK_SIZE
gcc   -o less1.exe *.o -llibcrypto -llibssl