gcc -c *.c -IC:\cygwin64\usr\include -I..\lib\XKCP -I..\lib\cryptocode -I..\lib\djbsort -I..\lib\randombytes -std=c99 -pedantic -Wall -Wextra -O3 -funroll-all-loops -march=native -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wshadow -Wno-newline-eof
gcc   -o perk-128-fast-5.exe *.o -llibcrypto -llibssl