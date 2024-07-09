gcc -c *.c -Wall -Wextra -Werror -pedantic -std=gnu99 -O3
gcc   -o dme1.exe *.o -llibcrypto -llibssl