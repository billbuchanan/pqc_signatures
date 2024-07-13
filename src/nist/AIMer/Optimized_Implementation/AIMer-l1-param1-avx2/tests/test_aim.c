// -----------------------------------------------------------------------------
// File Name   : test_aim.c
// Description : 
// SPDX-License-Identifier: MIT
// -----------------------------------------------------------------------------

#include "aim.h"
#include <stdio.h>

int main()
{
  int i;

  // Inverted order compared to Sage!
  uint8_t pt[16] =
    {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
     0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
  uint8_t ct_expected[16] =
    {0x32, 0x40, 0x74, 0x0c, 0x05, 0x8f, 0x57, 0x4e,
     0x7c, 0xc3, 0x06, 0x6e, 0xe5, 0x54, 0xff, 0xc7};
  uint8_t iv[16] =
    {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
     0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

  uint8_t ct[16] = {0,};

  aim(pt, iv, ct);

  printf("PLAINTEXT                 : ");
  for (i = (int)sizeof(pt) - 1; i >= 0; i--)
  {
    printf("%02x", pt[i]);
  }
  printf("\n");

  printf("IV                        : ");
  for (i = (int)sizeof(iv) - 1; i >= 0; i--)
  {
    printf("%02x", iv[i]);
  }
  printf("\n");

  printf("CIPHERTEXT                : ");
  for (i = (int)sizeof(ct) - 1; i >= 0; i--)
  {
    printf("%02x", ct[i]);

    if (ct[i] != ct_expected[i])
    {
      printf("\nError!\n");
      return 0;
    }
  }  
  printf("\nPassed!\n");

  // For  extracting test data -------------------------------------------------
  // printf("CIPHERTEXT(little endian) : ");
  // for (i = 0; i < (int)sizeof(ct) - 1; i++)
  // {
  //   printf("0x%02x, ", ct[i]);
  // }
  // printf("0x%02x\n", ct[i]);
  // For  extracting test data -------------------------------------------------

  return 0;
}
