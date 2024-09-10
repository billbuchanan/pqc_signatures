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
  uint8_t pt[24] =
    {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
     0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
     0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
  uint8_t ct_expected[24] =
    {0x97, 0xd8, 0x14, 0x92, 0x04, 0xc6, 0xe9, 0xa2,
     0x1a, 0x0b, 0xdf, 0xfa, 0xb2, 0x68, 0x75, 0xc3,
     0x9a, 0x3c, 0x77, 0x64, 0xc3, 0x12, 0xc2, 0xbd};
  uint8_t iv[24] =
    {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
     0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
     0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

  uint8_t ct[24] = {0,};

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
