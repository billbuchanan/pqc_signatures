gcc -c  PQCgenKAT_sign.c field/field128.c aim128.c hash.c api.c aimer_instances.c aimer.c tree.c aimer_internal.c rng.c
gcc   -o aimer5.exe *.o -llibcrypto -llibssl -l:libshake.a