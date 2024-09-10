gcc -c *.c -I..\include -D CATEGORY_5 -D PK_SIZE
gcc   -o less5.exe *.o -llibcrypto -llibssl