/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../randombytes.h"
#include "../fips202.h"
#include "../params.h"
#include "../sign.h"
#include "../poly.h"
#include "../polyvec.h"
#include "../polymatrix.h"
#include "../packing.h"

#define MLEN 32
#define NVECTORS 1000

void randombytes(uint8_t *out, size_t outlen)
{
  unsigned int i;
  uint8_t buf[8];
  static uint64_t ctr = 0;

  for (i = 0; i < 8; ++i)
    buf[i] = ctr >> 8 * i;

  ctr++;
  shake128(out, outlen, buf, 8);
}

int main(void)
{
  unsigned int i, j, k, l;
  uint8_t pk[CRYPTO_EAGLESIGN_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_EAGLESIGN_SECRETKEYBYTES];
  uint8_t sig[CRYPTO_EAGLESIGN_BYTES];
  uint8_t m[MLEN];
  uint8_t seed[CRHBYTES];
  uint8_t buf[CRYPTO_EAGLESIGN_SECRETKEYBYTES];
  size_t siglen;
  poly c, tmp;
  polyvecl A[K], G[L], D[K], G_INV[L], Y1;
  polyveck Y2;
  uint16_t nonce = 0;

  for (i = 0; i < NVECTORS; ++i)
  {
    printf("count = %u\n", i);

    randombytes(m, MLEN);
    printf("m = ");
    for (j = 0; j < MLEN; ++j)
      printf("%02x", m[j]);
    printf("\n");

    crypto_sign_keypair(pk, sk);
    shake256(buf, 32, pk, CRYPTO_EAGLESIGN_PUBLICKEYBYTES);
    printf("pk = ");
    for (j = 0; j < 32; ++j)
      printf("%02x", buf[j]);
    printf("\n");
    shake256(buf, 32, sk, CRYPTO_EAGLESIGN_SECRETKEYBYTES);
    printf("sk = ");
    for (j = 0; j < 32; ++j)
      printf("%02x", buf[j]);
    printf("\n");

    crypto_sign_signature(sig, &siglen, m, MLEN, sk);
    shake256(buf, 32, sig, CRYPTO_EAGLESIGN_BYTES);
    printf("sig = ");
    for (j = 0; j < 32; ++j)
      printf("%02x", buf[j]);
    printf("\n");

    if (crypto_sign_verify(sig, siglen, m, MLEN, pk))
      fprintf(stderr, "Signature verification failed!\n");

    randombytes(seed, sizeof(seed));
    printf("seed = ");
    for (j = 0; j < sizeof(seed); ++j)
      printf("%02X", seed[j]);
    printf("\n");

    polyvec_matrix_expand(A, seed);
    printf("A = ([");
    for (j = 0; j < K; ++j)
    {
      for (k = 0; k < L; ++k)
      {
        for (l = 0; l < N; ++l)
        {
          printf("%8d", A[j].vec[k].coeffs[l]);
          if (l < N - 1)
            printf(", ");
          else if (k < L - 1)
            printf("], [");
          else if (j < K - 1)
            printf("];\n     [");
          else
            printf("])\n");
        }
      }
    }

    polymatrix_k_l_expand(D, seed);
    polymatrix_invntt_tomont_k_l(D);

    if (polyvecl_chknorms(&D[0], LOGETAD))
      fprintf(stderr, "ERROR in polymatrix_invntt_tomont_k_l!\n");

    printf("D = ([");
    for (j = 0; j < K; ++j)
    {
      for (k = 0; k < L; ++k)
      {
        for (l = 0; l < N; ++l)
        {
          printf("%8d", D[j].vec[k].coeffs[l]);
          if (l < N - 1)
            printf(", ");
          else if (k < L - 1)
            printf("], [");
          else if (j < K - 1)
            printf("];\n     [");
          else
            printf("])\n");
        }
      }
    }

    polymatrix_l_expand_invertible(G_INV, G, seed);
    polymatrix_invntt_tomont_l_l(G);

    if (polyvecl_chknorms(&G[0], LOGETAG))
      fprintf(stderr, "ERROR in polymatrix_invntt_tomont_l_l!\n");

    polyG_pack(buf, &G[0].vec[0], LOGETAG);
    polyG_unpack(&tmp, buf, LOGETAG);

    for (j = 0; j < N; ++j)
      if (tmp.coeffs[j] != G[0].vec[0].coeffs[j])
        fprintf(stderr, "ERROR in polyG_(un)pack!\n");

    printf("G = ([");
    for (j = 0; j < L; ++j)
    {
      for (k = 0; k < L; ++k)
      {
        for (l = 0; l < N; ++l)
        {
          printf("%8d", G[j].vec[k].coeffs[l]);
          if (l < N - 1)
            printf(", ");
          else if (k < L - 1)
            printf("], [");
          else if (j < K - 1)
            printf("];\n     [");
          else
            printf("])\n");
        }
      }
    }
  }
  return 0;
}
