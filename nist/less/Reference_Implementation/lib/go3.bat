gcc -c *.c -I..\include -D CATEGORY_3 -D PK_SIZE
gcc   -o less3.exe *.o -llibcrypto -llibssl