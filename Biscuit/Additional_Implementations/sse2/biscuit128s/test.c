#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "biscuit.h"
#include "params.h"

#ifndef FIXED_PARAMS
static const params_t params[1] = { MAKE_PARAMS (
    SEC_LEVEL,
    NB_ITERATIONS,
    NB_PARTIES,
    FIELD_SIZE,
    NB_VARIABLES,
    NB_EQUATIONS,
    DEGREE
) };
#endif

static void
memprint_ (const uint8_t *src, size_t n)
{
  while (n--)
    printf ("%02x", *src++);
}

static void
memprint (const uint8_t *src, size_t n)
{
  memprint_ (src, n);
  printf ("\n");
}

static void
memrand (uint8_t *dst, size_t n)
{
  while (n--)
    *dst++ = rand ();
}

int
main (void)
{
  int status;

  unsigned int i;

  uint8_t pk[PUBKEY_BYTES], sk[PRIVKEY_BYTES];
  uint8_t sig[SIGNATURE_BYTES];
  uint8_t msg[128];
  uint64_t msglen;

  srand (0);

  printf ("params: ");
  printf ("%d %d %d %d %d %d %d\n",
          SEC_LEVEL, NB_ITERATIONS, NB_PARTIES,
          FIELD_SIZE, NB_VARIABLES, NB_EQUATIONS, DEGREE);

  memrand (sk, SEC_LEVEL >> 2);
  keygen (sk, pk, sk, params);
  printf ("keygen (sk %d bytes, pk %d bytes):\n", PRIVKEY_BYTES, PUBKEY_BYTES);
  memprint (sk, PRIVKEY_BYTES);
  memprint (pk, PUBKEY_BYTES);

  msglen = sizeof msg;
  for (i = 0; i < msglen; i++)
    msg[i] = i;

  printf ("sign: ");
  memrand (sig, SEC_LEVEL >> 2);
  sign (sig, msg, msglen, sk, NULL, params);
  printf ("%d bytes\n", SIGNATURE_BYTES);
  memprint_ (sig, 3 * SEC_LEVEL >> 2);
  printf ("...\n");

  printf ("verif: ");
  status = verify (sig, msg, msglen, pk, params);
  printf ("%s\n", status == 0 ? "ok" : "ko");

  return status;
}
