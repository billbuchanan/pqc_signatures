#include "compress.h"

/* Hashing of an array */
void hashArray(uint8_t *hash_out, const uint8_t *array_in, const uint64_t length)
{
  Keccak(1600-4*LAMBDA, 4*LAMBDA, array_in, length, 0x06, hash_out, LAMBDA/4);
}

/* Compress of an array of field elements */
void compressArray(uint8_t *carr_out, const uint64_t* arr_in, const int arr_size)
{
  int i,j=0;
  uint64_t s;
  for(i=0;i<arr_size;i++)
    {
      s=(arr_in[i]);
      carr_out[j++]=(uint8_t)s;
      carr_out[j++]=(uint8_t)(s>>8);
      carr_out[j++]=(uint8_t)(s>>16);
      carr_out[j++]=(uint8_t)(s>>24);
    }
}

/* Decompress of an array of field elements */
void decompressArray(uint64_t *darr_out, const uint8_t* arr_in, const int arr_size)
{
  int i,j=0;
  uint64_t s;
  for(i=0;i<arr_size;i++)
    {
      s=((uint64_t)arr_in[j++]);
      s+=((uint64_t)arr_in[j++]<<8);
      s+=((uint64_t)arr_in[j++]<<16);
      s+=((uint64_t)arr_in[j++]<<24);
      darr_out[i]=s;
    }
}
