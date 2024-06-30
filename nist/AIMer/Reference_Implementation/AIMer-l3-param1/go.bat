gcc -c  PQCgenKAT_sign.c field/field192.c aim192.c hash.c api.c aimer_instances.c aimer.c tree.c aimer_internal.c rng.c
gcc   -o aimer1.exe *.o -llibcrypto -llibssl -l:libshake.a