gcc -c aes\*.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc -c sha3\*.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc -c rans\*.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc -c sampling\*.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc -c normaldist\*.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc -c *.c -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
gcc   -o hufu3.exe *.o -llibcrypto -llibssl -lm -lssl -lcrypto