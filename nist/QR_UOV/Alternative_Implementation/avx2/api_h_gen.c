#include <stdio.h>
#include "qruov.h"

char * format =
  "#pragma once\n"
  "\n"
  "#include \"qruov.h\"\n"
  "\n"
  "//  Set these three values apropriately for your algorithm\n"
  "#define CRYPTO_SECRETKEYBYTES %d\n"
  "#define CRYPTO_PUBLICKEYBYTES %d\n"
  "#define CRYPTO_BYTES          %d\n"
  "\n"
  "// Change the algorithm name\n"
  "#define CRYPTO_ALGNAME \"%s\"\n"
  "\n"
  "int\n"
  "crypto_sign_keypair(unsigned char *pk, unsigned char *sk);\n"
  "\n"
  "int\n"
  "crypto_sign(unsigned char *sm, unsigned long long *smlen,\n"
  "            const unsigned char *m, unsigned long long mlen,\n"
  "            const unsigned char *sk);\n"
  "\n"
  "int\n"
  "crypto_sign_open(unsigned char *m, unsigned long long *mlen,\n"
  "                 const unsigned char *sm, unsigned long long smlen,\n"
  "                 const unsigned char *pk);\n"
;

int main(){
  printf(format,
    (QRUOV_SEED_LEN*2),
    (QRUOV_SEED_LEN+BITS2BYTES(QRUOV_ceil_log_2_q*QRUOV_L*((QRUOV_M*(QRUOV_M+1))/2)*QRUOV_m)),
    (QRUOV_SALT_LEN+BITS2BYTES(QRUOV_ceil_log_2_q*QRUOV_n)),
    QRUOV_ALGNAME
  ) ;
  return 0 ;
}
