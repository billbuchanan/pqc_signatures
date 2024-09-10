// -----------------------------------------------------------------------------
// File Name   : test_sign.c
// Description : 
// SPDX-License-Identifier: MIT
// -----------------------------------------------------------------------------

#include "aimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
  int ret = 0;
  const char *message = "TestMessage";

  aimer_publickey_t  public_key;
  aimer_privatekey_t private_key;

  uint8_t* signature = malloc(AIMER_MAX_SIGNATURE_SIZE);
  size_t  signature_len = 0;
  int param = 0;

  param = AIMER_L1_PARAM1;

  for (int i = 0; i < 4; i++)
  {
    printf("Test parameter %d\n", i + 1);
    ret = aimer_keygen(param++, &public_key, &private_key);

    ret = aimer_sign(&public_key, &private_key, (const uint8_t*)message,
                   strlen(message), signature, &signature_len);

    if(ret != 0)
    {
      printf("Sign Failure\n");
    }
    else
    {
      printf("Signature Size: [%ld]\n", signature_len);
    }

    ret = aimer_verify(&public_key, signature, signature_len,
                      (const uint8_t*)message, strlen(message));

    if(ret == 0)
    {
      printf("Verification Success\n\n");
    }
    else
    {
      printf("Verification Failure[%d]\n\n", ret);
    }
  }
  
  free(signature);
  return ret;
}
