#include "fips202x4.h"

__m256i rol64x4(__m256i A, int offset){
   __m256i ts, tp;
   ts = _mm256_slli_epi64 (A, offset);
   tp = _mm256_srli_epi64 (A, 64-offset);
   ts = _mm256_xor_si256 (ts, tp);
   return ts;
}

static const int rhotates[5][5] = {
    {  0,  1, 62, 28, 27 },
    { 36, 44,  6, 55, 20 },
    {  3, 10, 43, 25, 39 },
    { 41, 45, 15, 21,  8 },
    { 18,  2, 61, 56, 14 }
};

static const uint64_t iotas[24] = {
  (uint64_t)0x0000000000000001ULL,
  (uint64_t)0x0000000000008082ULL,
  (uint64_t)0x800000000000808aULL,
  (uint64_t)0x8000000080008000ULL,
  (uint64_t)0x000000000000808bULL,
  (uint64_t)0x0000000080000001ULL,
  (uint64_t)0x8000000080008081ULL,
  (uint64_t)0x8000000000008009ULL,
  (uint64_t)0x000000000000008aULL,
  (uint64_t)0x0000000000000088ULL,
  (uint64_t)0x0000000080008009ULL,
  (uint64_t)0x000000008000000aULL,
  (uint64_t)0x000000008000808bULL,
  (uint64_t)0x800000000000008bULL,
  (uint64_t)0x8000000000008089ULL,
  (uint64_t)0x8000000000008003ULL,
  (uint64_t)0x8000000000008002ULL,
  (uint64_t)0x8000000000000080ULL,
  (uint64_t)0x000000000000800aULL,
  (uint64_t)0x800000008000000aULL,
  (uint64_t)0x8000000080008081ULL,
  (uint64_t)0x8000000000008080ULL,
  (uint64_t)0x0000000080000001ULL,
  (uint64_t)0x8000000080008008ULL
};

void sha3x4_permut1600(__m256i A[5][5])
{
     __m256i tc[5], td[5], tp[5];
     __m256i T[5][5];
     int y, ir;
     
     for(ir=0; ir<24; ir++)
     {
         //----------theta-------------
         tc[0] = _mm256_xor_si256 (A[0][0], A[1][0]);
         tc[1] = _mm256_xor_si256 (A[0][1], A[1][1]);
         tc[2] = _mm256_xor_si256 (A[0][2], A[1][2]);
         tc[3] = _mm256_xor_si256 (A[0][3], A[1][3]);
         tc[4] = _mm256_xor_si256 (A[0][4], A[1][4]);
         for (y = 2; y < 5; y++) {
             tc[0] = _mm256_xor_si256 (tc[0], A[y][0]);
             tc[1] = _mm256_xor_si256 (tc[1], A[y][1]);
             tc[2] = _mm256_xor_si256 (tc[2], A[y][2]);
             tc[3] = _mm256_xor_si256 (tc[3], A[y][3]);
             tc[4] = _mm256_xor_si256 (tc[4], A[y][4]);
         }   

         td[0] = rol64x4(tc[1], 1);
         td[1] = rol64x4(tc[2], 1);
         td[2] = rol64x4(tc[3], 1);
         td[3] = rol64x4(tc[4], 1);
         td[4] = rol64x4(tc[0], 1);

         td[0] = _mm256_xor_si256 (td[0], tc[4]);
         td[1] = _mm256_xor_si256 (td[1], tc[0]);
         td[2] = _mm256_xor_si256 (td[2], tc[1]);
         td[3] = _mm256_xor_si256 (td[3], tc[2]);
         td[4] = _mm256_xor_si256 (td[4], tc[3]);         
         for (y = 0; y < 5; y++) {
             A[y][0] = _mm256_xor_si256 (td[0], A[y][0]);
             A[y][1] = _mm256_xor_si256 (td[1], A[y][1]);
             A[y][2] = _mm256_xor_si256 (td[2], A[y][2]);
             A[y][3] = _mm256_xor_si256 (td[3], A[y][3]);
             A[y][4] = _mm256_xor_si256 (td[4], A[y][4]);
         }
        //------------rho-------------
         for (y = 0; y < 5; y++) {
              T[y][0] = rol64x4(A[y][0], rhotates[y][0]);
              T[y][1] = rol64x4(A[y][1], rhotates[y][1]);
              T[y][2] = rol64x4(A[y][2], rhotates[y][2]);
              T[y][3] = rol64x4(A[y][3], rhotates[y][3]);
              T[y][4] = rol64x4(A[y][4], rhotates[y][4]);
         }
         //------------pi-chi-----------
         tc[0] = _mm256_andnot_si256 (T[1][1], T[2][2]);
         tc[1] = _mm256_andnot_si256 (T[2][2], T[3][3]);
         tc[2] = _mm256_andnot_si256 (T[3][3], T[4][4]);
         tc[3] = _mm256_andnot_si256 (T[4][4], T[0][0]);
         tc[4] = _mm256_andnot_si256 (T[0][0], T[1][1]);
         A[0][0] = _mm256_xor_si256 (T[0][0], tc[0]);     
         A[0][1] = _mm256_xor_si256 (T[1][1], tc[1]);    
         A[0][2] = _mm256_xor_si256 (T[2][2], tc[2]);    
         A[0][3] = _mm256_xor_si256 (T[3][3], tc[3]);   
         A[0][4] = _mm256_xor_si256 (T[4][4], tc[4]);

         tc[0] = _mm256_andnot_si256 (T[1][4], T[2][0]);
         tc[1] = _mm256_andnot_si256 (T[2][0], T[3][1]);
         tc[2] = _mm256_andnot_si256 (T[3][1], T[4][2]);
         tc[3] = _mm256_andnot_si256 (T[4][2], T[0][3]);
         tc[4] = _mm256_andnot_si256 (T[0][3], T[1][4]);
         A[1][0] = _mm256_xor_si256 (T[0][3], tc[0]);     
         A[1][1] = _mm256_xor_si256 (T[1][4], tc[1]);    
         A[1][2] = _mm256_xor_si256 (T[2][0], tc[2]);    
         A[1][3] = _mm256_xor_si256 (T[3][1], tc[3]);   
         A[1][4] = _mm256_xor_si256 (T[4][2], tc[4]);

         tc[0] = _mm256_andnot_si256 (T[1][2], T[2][3]);
         tc[1] = _mm256_andnot_si256 (T[2][3], T[3][4]);
         tc[2] = _mm256_andnot_si256 (T[3][4], T[4][0]);
         tc[3] = _mm256_andnot_si256 (T[4][0], T[0][1]);
         tc[4] = _mm256_andnot_si256 (T[0][1], T[1][2]);
         A[2][0] = _mm256_xor_si256 (T[0][1], tc[0]);     
         A[2][1] = _mm256_xor_si256 (T[1][2], tc[1]);    
         A[2][2] = _mm256_xor_si256 (T[2][3], tc[2]);    
         A[2][3] = _mm256_xor_si256 (T[3][4], tc[3]);   
         A[2][4] = _mm256_xor_si256 (T[4][0], tc[4]);

         tc[0] = _mm256_andnot_si256 (T[1][0], T[2][1]);
         tc[1] = _mm256_andnot_si256 (T[2][1], T[3][2]);
         tc[2] = _mm256_andnot_si256 (T[3][2], T[4][3]);
         tc[3] = _mm256_andnot_si256 (T[4][3], T[0][4]);
         tc[4] = _mm256_andnot_si256 (T[0][4], T[1][0]);
         A[3][0] = _mm256_xor_si256 (T[0][4], tc[0]);     
         A[3][1] = _mm256_xor_si256 (T[1][0], tc[1]);    
         A[3][2] = _mm256_xor_si256 (T[2][1], tc[2]);    
         A[3][3] = _mm256_xor_si256 (T[3][2], tc[3]);   
         A[3][4] = _mm256_xor_si256 (T[4][3], tc[4]);

         tc[0] = _mm256_andnot_si256 (T[1][3], T[2][4]);
         tc[1] = _mm256_andnot_si256 (T[2][4], T[3][0]);
         tc[2] = _mm256_andnot_si256 (T[3][0], T[4][1]);
         tc[3] = _mm256_andnot_si256 (T[4][1], T[0][2]);
         tc[4] = _mm256_andnot_si256 (T[0][2], T[1][3]);
         A[4][0] = _mm256_xor_si256 (T[0][2], tc[0]);     
         A[4][1] = _mm256_xor_si256 (T[1][3], tc[1]);    
         A[4][2] = _mm256_xor_si256 (T[2][4], tc[2]);    
         A[4][3] = _mm256_xor_si256 (T[3][0], tc[3]);   
         A[4][4] = _mm256_xor_si256 (T[4][1], tc[4]);
         //-------------iota-----------------
         tc[0] = _mm256_set_epi64x (iotas[ir], iotas[ir], iotas[ir], iotas[ir]);
         A[0][0] = _mm256_xor_si256 (A[0][0], tc[0]); 
      }
}

//-----------shake256x4, only for 32byte seed.------------------------------------
void shake256x4_absorb(__m256i A[5][5], const uint8_t seed[32])
{
     uint64_t tp64, t1, t2, t3, t4;
     for(int i = 0; i<5; i++)
     {
         for(int j = 0; j<5; j++)
              A[i][j] = _mm256_setzero_si256(); 
     }     
     memcpy(&tp64, seed, 8);
     tp64 = tp64 << 8;
     t1 = tp64 ^ 0x00;
     t2 = tp64 ^ 0x01;
     t3 = tp64 ^ 0x02;
     t4 = tp64 ^ 0x03;
     A[0][0] = _mm256_set_epi64x (t4, t3, t2, t1);

     for(int i = 1; i<4; i++)
     {
         memcpy(&tp64, seed+8*i-1, 8);
         A[0][i] = _mm256_set_epi64x (tp64, tp64, tp64, tp64);
     }

     tp64 = (uint64_t)(seed[31]) ^ 0x1f00;
     A[0][4] = _mm256_set_epi64x (tp64, tp64, tp64, tp64);     
     A[3][1] = _mm256_set_epi64x (0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000);    
}


void shake256x4_squeeze_1088(__m256i A[5][5], uint8_t s0[136], uint8_t s1[136], uint8_t s2[136], uint8_t s3[136])
{    
     sha3x4_permut1600(A);

     __m256i t0, t1, t2, t3, out;
     uint64_t t64[4];
   
     t0 = _mm256_unpacklo_epi64 (A[0][0], A[0][1]);  //s0, s2
     t1 = _mm256_unpackhi_epi64 (A[0][0], A[0][1]);  //s1, s3
     t2 = _mm256_unpacklo_epi64 (A[0][2], A[0][3]);  //s0, s2
     t3 = _mm256_unpackhi_epi64 (A[0][2], A[0][3]);  //s1, s3 
     out = _mm256_permute2f128_si256 (t0, t2, 0x20); 
     _mm256_storeu_si256 ((__m256i *) s0, out);
     out= _mm256_permute2f128_si256 (t0, t2, 0x31); 
     _mm256_storeu_si256 ((__m256i *) s2, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x20);
     _mm256_storeu_si256 ((__m256i *) s1, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x31);
     _mm256_storeu_si256 ((__m256i *) s3, out);
     s0 = s0+32;
     s1 = s1+32;
     s2 = s2+32;
     s3 = s3+32;

     t0 = _mm256_unpacklo_epi64 (A[0][4], A[1][0]);  
     t1 = _mm256_unpackhi_epi64 (A[0][4], A[1][0]);  
     t2 = _mm256_unpacklo_epi64 (A[1][1], A[1][2]);  
     t3 = _mm256_unpackhi_epi64 (A[1][1], A[1][2]);   
     out = _mm256_permute2f128_si256 (t0, t2, 0x20); 
     _mm256_storeu_si256 ((__m256i *) s0, out);
     out= _mm256_permute2f128_si256 (t0, t2, 0x31); 
     _mm256_storeu_si256 ((__m256i *) s2, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x20);
     _mm256_storeu_si256 ((__m256i *) s1, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x31);
     _mm256_storeu_si256 ((__m256i *) s3, out);
     s0 = s0+32;
     s1 = s1+32;
     s2 = s2+32;
     s3 = s3+32;
     

     t0 = _mm256_unpacklo_epi64 (A[1][3], A[1][4]);  
     t1 = _mm256_unpackhi_epi64 (A[1][3], A[1][4]);  
     t2 = _mm256_unpacklo_epi64 (A[2][0], A[2][1]);  
     t3 = _mm256_unpackhi_epi64 (A[2][0], A[2][1]);   
     out = _mm256_permute2f128_si256 (t0, t2, 0x20); 
     _mm256_storeu_si256 ((__m256i *) s0, out);
     out= _mm256_permute2f128_si256 (t0, t2, 0x31); 
     _mm256_storeu_si256 ((__m256i *) s2, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x20);
     _mm256_storeu_si256 ((__m256i *) s1, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x31);
     _mm256_storeu_si256 ((__m256i *) s3, out);
     s0 = s0+32;
     s1 = s1+32;
     s2 = s2+32;
     s3 = s3+32;

     t0 = _mm256_unpacklo_epi64 (A[2][2], A[2][3]);  
     t1 = _mm256_unpackhi_epi64 (A[2][2], A[2][3]);  
     t2 = _mm256_unpacklo_epi64 (A[2][4], A[3][0]);  
     t3 = _mm256_unpackhi_epi64 (A[2][4], A[3][0]);   
     out = _mm256_permute2f128_si256 (t0, t2, 0x20); 
     _mm256_storeu_si256 ((__m256i *) s0, out);
     out= _mm256_permute2f128_si256 (t0, t2, 0x31); 
     _mm256_storeu_si256 ((__m256i *) s2, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x20);
     _mm256_storeu_si256 ((__m256i *) s1, out);
     out = _mm256_permute2f128_si256 (t1, t3, 0x31);
     _mm256_storeu_si256 ((__m256i *) s3, out);
     s0 = s0+32;
     s1 = s1+32;
     s2 = s2+32;
     s3 = s3+32;

     _mm256_store_si256 ((__m256i *) t64, A[3][1]);
     memcpy(s0, t64, 8);
     memcpy(s1, t64+1, 8);
     memcpy(s2, t64+2, 8);
     memcpy(s3, t64+3, 8);         
}

void shake256x4_squeeze(__m256i A[5][5], uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3, unsigned long long outlen){

     uint8_t s0[136], s1[136], s2[136], s3[136];
     while(outlen > 0){
         shake256x4_squeeze_1088(A, s0, s1, s2, s3);
         if(outlen >=136){ 
               memcpy(out0, s0, 136);
               memcpy(out1, s1, 136);
               memcpy(out2, s2, 136);
               memcpy(out3, s3, 136);
               out0 = out0 + 136;
               out1 = out1 + 136;
               out2 = out2 + 136;
               out3 = out3 + 136;
               outlen = outlen - 136;
         }else{
               memcpy(out0, s0, outlen);
               memcpy(out1, s1, outlen);
               memcpy(out2, s2, outlen);
               memcpy(out3, s3, outlen);
               outlen = 0;
               break;
         }
     }
}

void shake256x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3, unsigned long long outlen, const uint8_t *input,  unsigned long long inlen){
        __m256i A[5][5];
        shake256x4_absorb(A, input);
        shake256x4_squeeze(A, out0, out1, out2, out3, outlen);
}











