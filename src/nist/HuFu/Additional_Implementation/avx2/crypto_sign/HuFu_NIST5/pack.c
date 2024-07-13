#include "pack.h"
#include "stdio.h"
#include"immintrin.h"
#include "util.h"

void pack_new(uint8_t *buf, const size_t buf_len,   const uint32_t *data, const size_t data_len){
        //buf_len = data_len*2+data_len/8;
        uint8_t *symbol = buf + data_len*2;
        for(int i = 0; i < (int)data_len; i++){
              buf[i*2+0] = (uint8_t) (data[i]);
              buf[i*2+1] = (uint8_t) (data[i] >> 8);
        }
        for(int i = 0; i < (int)data_len/8; i++){
            for(int j = 0; j < 8; j++){
                 symbol[i] = ((data[i*8+0]>>9)&0x80) ^ ((data[i*8+1]>>10)&0x40) ^ ((data[i*8+2]>>11)&0x20) ^ ((data[i*8+3]>>12)&0x10) ^ 
                             ((data[i*8+4]>>13)&0x08) ^ ((data[i*8+5]>>14)&0x04) ^ ((data[i*8+6]>>15)&0x02) ^ ((data[i*8+7]>>16)&0x01);
            }
        }

}
void unpack_avx2(uint32_t *data, const size_t data_len,  const uint8_t *buf,const uint8_t *siged,const size_t buf_len){
        __m128i vecbuf;
        __m256i vecdata;
        __m256i vecsign;
        __m256i left = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        __m256i masksign = _mm256_set1_epi32(0x80000000);
        // uint8_t *symbol = (uint8_t *)buf + data_len * 2;
        for (size_t i = 0; i < data_len / 8; i++)
        {
            vecbuf = _mm_loadu_si128((__m128i *)(buf+16*i));
            vecdata = _mm256_cvtepu16_epi32(vecbuf);
            vecsign = _mm256_set1_epi8(siged[i]);
            vecsign = _mm256_sllv_epi32(vecsign, left);
            vecsign = _mm256_and_si256(vecsign, masksign);
            vecsign = _mm256_srai_epi32(vecsign, 15);
            vecdata = _mm256_or_si256(vecdata, vecsign);
            _mm256_storeu_si256((__m256i *)(data + 8 * i), vecdata);
        }
}

void mat_mul_unpack_avx2(int32_t * C, const uint8_t * A,const uint8_t *A1, const int32_t * B, int l1, int l2, int l3){
    int32_t *realA = malloc(l1 * l2 * sizeof(int32_t));
    unpack_avx2((uint32_t*)realA,l1*l2,A,A1,(l1*l2*17)>>3);
    mat_mul_avx2(C,realA,B,l1,l2,l3);
    free(realA);
}

// pack and unpack mat R, in which each element has two bits
void pack_mat_r(uint8_t *buf, const int32_t *data)
{
    for (size_t i = 0; i < PARAM_R_BYTES; i++)
    {
        buf[i] = ((data[0] & 0x03) << 6) | ((data[1] & 0x03) << 4) | ((data[2] & 0x03) << 2) | (data[3] & 0x03);
        data += 4;
    }
}

void unpack_mat_r_avx2(int32_t *data, const uint8_t *buf, int buf_len)
{
    for (int i = 0; i < buf_len/32; i++)
    {
        __m256i a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3, b4, b5, b6, b7;
        a0 =  _mm256_loadu_si256((__m256i*)(buf));
        a1 =  _mm256_slli_epi16 (a0, 2);
        a2 =  _mm256_slli_epi16 (a0, 4);
        a3 =  _mm256_slli_epi16 (a0, 6);
        a4 =  _mm256_slli_epi16 (a0, 8);
        a5 =  _mm256_slli_epi16 (a0, 10);
        a6 =  _mm256_slli_epi16 (a0, 12);          
        a7 =  _mm256_slli_epi16 (a0, 14); 

        a0 = _mm256_srai_epi16 (a0, 14);
        a1 = _mm256_srai_epi16 (a1, 14);
        a2 = _mm256_srai_epi16 (a2, 14);
        a3 = _mm256_srai_epi16 (a3, 14);
        a4 = _mm256_srai_epi16 (a4, 14);
        a5 = _mm256_srai_epi16 (a5, 14);
        a6 = _mm256_srai_epi16 (a6, 14);
        a7 = _mm256_srai_epi16 (a7, 14);

        b0 = _mm256_unpacklo_epi16 (a0, a1);
        b1 = _mm256_unpackhi_epi16 (a0, a1);
        b2 = _mm256_unpacklo_epi16 (a2, a3);
        b3 = _mm256_unpackhi_epi16 (a2, a3); 
        b4 = _mm256_unpacklo_epi16 (a4, a5);
        b5 = _mm256_unpackhi_epi16 (a4, a5);
        b6 = _mm256_unpacklo_epi16 (a6, a7);
        b7 = _mm256_unpackhi_epi16 (a6, a7);

        a0 = _mm256_unpacklo_epi32 (b0, b2);
        a1 = _mm256_unpackhi_epi32 (b0, b2);
        a2 = _mm256_unpacklo_epi32 (b1, b3);
        a3 = _mm256_unpackhi_epi32 (b1, b3);
        a4 = _mm256_unpacklo_epi32 (b4, b6);
        a5 = _mm256_unpackhi_epi32 (b4, b6);
        a6 = _mm256_unpacklo_epi32 (b5, b7);
        a7 = _mm256_unpackhi_epi32 (b5, b7);

        b0 = _mm256_unpacklo_epi64 (a4, a0);
        b1 = _mm256_unpackhi_epi64 (a4, a0);
        b2 = _mm256_unpacklo_epi64 (a5, a1);
        b3 = _mm256_unpackhi_epi64 (a5, a1);
        b4 = _mm256_unpacklo_epi64 (a6, a2);
        b5 = _mm256_unpackhi_epi64 (a6, a2);
        b6 = _mm256_unpacklo_epi64 (a7, a3);
        b7 = _mm256_unpackhi_epi64 (a7, a3);

        a0 = _mm256_permute2f128_si256 (b0, b1, 0x20);
        a1 = _mm256_permute2f128_si256 (b0, b1, 0x31);
        a2 = _mm256_permute2f128_si256 (b2, b3, 0x20);
        a3 = _mm256_permute2f128_si256 (b2, b3, 0x31);
        a4 = _mm256_permute2f128_si256 (b4, b5, 0x20);
        a5 = _mm256_permute2f128_si256 (b4, b5, 0x31);
        a6 = _mm256_permute2f128_si256 (b6, b7, 0x20);
        a7 = _mm256_permute2f128_si256 (b6, b7, 0x31);
       
         
        __m128i c0, c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15;
        c0 = _mm256_extractf128_si256(a0, 0);
        c1 = _mm256_extractf128_si256(a0, 1);
        c2 = _mm256_extractf128_si256(a2, 0);
        c3 = _mm256_extractf128_si256(a2, 1);
        c4 = _mm256_extractf128_si256(a4, 0);
        c5 = _mm256_extractf128_si256(a4, 1);
        c6 = _mm256_extractf128_si256(a6, 0);
        c7 = _mm256_extractf128_si256(a6, 1);
        c8 = _mm256_extractf128_si256(a1, 0);
        c9 = _mm256_extractf128_si256(a1, 1);
        c10 = _mm256_extractf128_si256(a3, 0);
        c11 = _mm256_extractf128_si256(a3, 1);
        c12 = _mm256_extractf128_si256(a5, 0);
        c13 = _mm256_extractf128_si256(a5, 1);
        c14 = _mm256_extractf128_si256(a7, 0);
        c15 = _mm256_extractf128_si256(a7, 1);
        b0 = _mm256_cvtepi16_epi32(c0);
        a0=_mm256_cvtepi16_epi32(c1);
        b1 = _mm256_cvtepi16_epi32(c2);
        a1=_mm256_cvtepi16_epi32(c3);
        b2 = _mm256_cvtepi16_epi32(c4);
        a2=_mm256_cvtepi16_epi32(c5);
        b3 = _mm256_cvtepi16_epi32(c6);
        a3=_mm256_cvtepi16_epi32(c7);
        b4 = _mm256_cvtepi16_epi32(c8);
        a4=_mm256_cvtepi16_epi32(c9);
        b5 = _mm256_cvtepi16_epi32(c10);
        a5=_mm256_cvtepi16_epi32(c11);
        b6 = _mm256_cvtepi16_epi32(c12);
        a6=_mm256_cvtepi16_epi32(c13);
        b7 = _mm256_cvtepi16_epi32(c14);
        a7=_mm256_cvtepi16_epi32(c15);
        _mm256_storeu_si256((__m256i *)data, b0);
        _mm256_storeu_si256((__m256i *) (data+8), a0); 
        _mm256_storeu_si256((__m256i *) (data+16), b1); 
        _mm256_storeu_si256((__m256i *) (data+24), a1); 
        _mm256_storeu_si256((__m256i *) (data+32), b2); 
        _mm256_storeu_si256((__m256i *) (data+40), a2); 
        _mm256_storeu_si256((__m256i *) (data+48), b3); 
        _mm256_storeu_si256((__m256i *) (data+56), a3); 
        _mm256_storeu_si256((__m256i *)(data+64), b4);
        _mm256_storeu_si256((__m256i *) (data+72), a4); 
        _mm256_storeu_si256((__m256i *) (data+80), b5); 
        _mm256_storeu_si256((__m256i *) (data+88), a5); 
        _mm256_storeu_si256((__m256i *) (data+96), b6); 
        _mm256_storeu_si256((__m256i *) (data+104), a6); 
        _mm256_storeu_si256((__m256i *) (data+112), b7); 
        _mm256_storeu_si256((__m256i *) (data+120), a7); 
        buf = buf + 32;
        data = data + 128;
    }
}