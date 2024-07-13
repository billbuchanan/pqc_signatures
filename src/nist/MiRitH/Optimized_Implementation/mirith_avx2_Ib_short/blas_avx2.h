/* Based on the public domain implementation in pqov-paper/src/avx2/ from
 * https://github.com/pqov/pqov-paper by Ward Beullens, Ming-Shing Chen,
 * Shih-Hao Hung, Matthias J. Kannwischer, Bo-Yuan Peng, Cheng-Jhih Shih,
 * and Bo-Yin Yang */

#ifndef MIRITH_DSS_BLAS_AVX2_H
#define MIRITH_DSS_BLAS_AVX2_H
/// @file blas_avx2.h
/// @brief Inlined functions for implementing basic linear algebra functions for AVX2 arch.
///

#include <stdint.h>
#include <immintrin.h>
#include "matrix_constants.h"


//////////////////////////////////////////////
////// new code //


/// Full multiplication
/// NOTE: assumes that in every byte the two nibbles are the same in b
/// \return a*b \in \F_16 for all 64 nibbles in the
__m256i gf16_mult_avx_compressed_2(const __m256i a, const __m256i b) {
    const __m256i mask_lvl2 = _mm256_load_si256((__m256i const *) (__gf16_mulbase +   32));
    const __m256i mask_lvl3 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*2));
    const __m256i mask_lvl4 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*3));
    const __m256i zero = _mm256_setzero_si256();

    __m256i low_lookup = b;
    __m256i high_lookup = _mm256_slli_epi16(low_lookup, 4);
    __m256i tmp1l = _mm256_slli_epi16(a, 7);
    __m256i tmp2h = _mm256_slli_epi16(a, 3);
    __m256i tmp_mul_0_1 = _mm256_blendv_epi8(zero, low_lookup , tmp1l);
    __m256i tmp_mul_0_2 = _mm256_blendv_epi8(zero, high_lookup, tmp2h);
    __m256i tmp = _mm256_xor_si256(tmp_mul_0_1, tmp_mul_0_2);
    __m256i tmp1;

    /// 1
    low_lookup = _mm256_shuffle_epi8(mask_lvl2, b);
    high_lookup = _mm256_slli_epi16(low_lookup, 4);
    tmp1l = _mm256_slli_epi16(a, 6);
    tmp2h = _mm256_slli_epi16(a, 2);
    tmp_mul_0_1 = _mm256_blendv_epi8(zero, low_lookup , tmp1l);
    tmp_mul_0_2 = _mm256_blendv_epi8(zero, high_lookup, tmp2h);
    tmp1 = _mm256_xor_si256(tmp_mul_0_1, tmp_mul_0_2);
    tmp  = _mm256_xor_si256(tmp, tmp1);

    /// 2
    low_lookup = _mm256_shuffle_epi8(mask_lvl3, b);
    high_lookup = _mm256_slli_epi16(low_lookup, 4);
    tmp1l = _mm256_slli_epi16(a, 5);
    tmp2h = _mm256_slli_epi16(a, 1);
    tmp_mul_0_1 = _mm256_blendv_epi8(zero, low_lookup , tmp1l);
    tmp_mul_0_2 = _mm256_blendv_epi8(zero, high_lookup, tmp2h);
    tmp1 = _mm256_xor_si256(tmp_mul_0_1, tmp_mul_0_2);
    tmp  = _mm256_xor_si256(tmp, tmp1);

    /// 3
    low_lookup = _mm256_shuffle_epi8(mask_lvl4, b);
    high_lookup = _mm256_slli_epi16(low_lookup, 4);
    tmp1l = _mm256_slli_epi16(a, 4);
    tmp_mul_0_1 = _mm256_blendv_epi8(zero, low_lookup , tmp1l);
    tmp_mul_0_2 = _mm256_blendv_epi8(zero, high_lookup, a );
    tmp1 = _mm256_xor_si256(tmp_mul_0_1, tmp_mul_0_2);
    tmp  = _mm256_xor_si256(tmp, tmp1);
    return tmp;
}


void gf16mat_new_core(uint8_t *__restrict c,
                      const uint8_t *__restrict__ a,
                      const unsigned int nr_cols_B,
                      const unsigned int column_A_bytes,
                      const unsigned int nr_cols_A,
                      const uint8_t *__restrict__ b) {
    const uint32_t nr_bytes_B_col = nr_cols_A >> 1;
    const uint32_t maxByteInAVX2 = (32 / column_A_bytes) * column_A_bytes;
    const uint32_t colsInAVX2A = (32 / column_A_bytes);
    const uint32_t nrA256 = ((column_A_bytes*nr_cols_A)+maxByteInAVX2-1) / maxByteInAVX2;
    const uint32_t nrB256 = nrA256;
    __uint128_t ret = 0;
    uint8_t bmasked[256] __attribute__((aligned(32))) = {0};

    // load the matrix A into registers
    __m256i Adata[nrA256], Bdata[nrB256];
    for (uint32_t i = 0; i < nrA256-1; ++i) {
        Adata[i] = _mm256_loadu_si256((__m256i *)(a + i*maxByteInAVX2));
    }
    Adata[nrA256-1] = _mm256_loadu_si256((__m256i *)(a + (nrA256-1)*maxByteInAVX2));

    for (uint32_t k = 0; k < nr_cols_B; ++k) {
        // load B
        __uint128_t b_data = *((__uint128_t *)(b+(nr_bytes_B_col * k)));
        __uint128_t b1 = b_data;
        __uint128_t b2 = (b_data >> 4);
        uint32_t base = 0;

        for (uint32_t j = 0; j < nr_bytes_B_col; ++j) {
            const uint8_t bytel = (b1>>(j*8))& 0xf;
            const uint8_t byteh = (b2>>(j*8))& 0xf;

            for (uint32_t i = 0; i < column_A_bytes; ++i) {
                bmasked[base + i] = bytel;
                bmasked[base + column_A_bytes + i] = byteh;
            }

            base += 2*column_A_bytes;
        }

        __m256i tmp = _mm256_setzero_si256();
        for (uint32_t i = 0; i < nrA256-1; ++i) {
            Bdata[i] = _mm256_loadu_si256((__m256i *)(bmasked + i*maxByteInAVX2));
            tmp ^= gf16_mult_avx_compressed_2(Adata[i], Bdata[i]);
        }

        Bdata[nrA256-1] = _mm256_loadu_si256((__m256i *)(bmasked + (nrB256-1)*maxByteInAVX2));
        tmp ^= gf16_mult_avx_compressed_2(Adata[nrB256-1], Bdata[nrB256-1]);

        // store and compute sum;
        _mm256_store_si256((__m256i *)bmasked, tmp);
        ret = 0;
        for (uint32_t i = 0; i < colsInAVX2A; i++) {
            ret ^= (*((__uint128_t *)(bmasked + i*column_A_bytes)));
        }

        for (uint32_t i = 0; i < column_A_bytes; i++) {
            c[k*column_A_bytes + i] ^= (uint8_t)(ret>>(i*8));
        }
    }
}



////////////128-bits functions///////////////
// 6 instructions
static inline __m128i linear_transform_8x8_128b( __m128i tab_l , __m128i tab_h , __m128i v , __m128i mask_f )
{
    return _mm_shuffle_epi8(tab_l,v&mask_f)^_mm_shuffle_epi8(tab_h,_mm_srli_epi16(v,4)&mask_f);
}

//////////////////////from blas_sse.h//////////////////////////////////////
static inline
__m128i _load_xmm( const uint8_t *a , unsigned _num_byte ) {
    uint8_t temp[32] __attribute__((aligned(32)));
    //assert( 16 >= _num_byte );
    //assert( 0 < _num_byte );
    for(unsigned i=0;i<_num_byte;i++) temp[i] = a[i];
    return _mm_load_si128((__m128i*)temp);
}

static inline
void loadu_xmm( __m128i *xmm_a, const uint8_t *a, unsigned _num_byte ) {
    unsigned n_16 = (_num_byte>>4);
    unsigned n_16_rem = _num_byte&0xf;
    while( n_16-- ) {
        xmm_a[0] = _mm_loadu_si128( (__m128i*)(a) );
        xmm_a++;
        a += 16;
    }
    if( n_16_rem ) xmm_a[0] = _load_xmm( a , n_16_rem );
}

static inline
void _store_xmm( uint8_t *a , unsigned _num_byte , __m128i data ) {
    uint8_t temp[32] __attribute__((aligned(32)));
    //assert( 16 >= _num_byte );
    //assert( 0 < _num_byte );
    _mm_store_si128((__m128i*)temp,data);
    for(unsigned i=0;i<_num_byte;i++) a[i] = temp[i];
}

static inline
void storeu_xmm( uint8_t *a , unsigned _num_byte , __m128i *xmm_a ) {
    unsigned n_16 = (_num_byte>>4);
    unsigned n_16_rem = _num_byte&0xf;
    while( n_16-- ) {
        _mm_storeu_si128( (__m128i*)a , xmm_a[0] );
        xmm_a++;
        a += 16;
    }
    if( n_16_rem ) _store_xmm( a , n_16_rem , xmm_a[0] );
}

static inline
void linearmap_8x8_sse( uint8_t * a , __m128i ml , __m128i mh , __m128i mask , unsigned _num_byte ) {
    unsigned n_16 = _num_byte>>4;
    unsigned rem = _num_byte&15;
    while( n_16--) {
        __m128i inp = _mm_loadu_si128( (__m128i*)(a) );
        __m128i r0 = linear_transform_8x8_128b( ml , mh , inp , mask );
        _mm_storeu_si128( (__m128i*)(a) , r0 );
        a += 16;
    }
    if( rem ) {
        __m128i inp = _load_xmm( a , rem );
        __m128i r0 = linear_transform_8x8_128b( ml , mh , inp , mask );
        _store_xmm( a , rem , r0 );
    }
}


static inline
void linearmap_8x8_accu_sse( uint8_t * accu_c, const uint8_t * a , __m128i ml , __m128i mh , __m128i mask , unsigned _num_byte ) {
    unsigned n_16 = _num_byte>>4;
    unsigned rem = _num_byte&15;
    while( n_16-- ) {
        __m128i inp = _mm_loadu_si128( (__m128i*)(a) );
        __m128i out = _mm_loadu_si128( (__m128i*)(accu_c) );
        __m128i r0 = linear_transform_8x8_128b( ml , mh , inp , mask );
        r0 ^= out;
        _mm_storeu_si128( (__m128i*)(accu_c) , r0 );
        a += 16;
        accu_c += 16;
    }
    if( rem ) {
        __m128i inp = _load_xmm( a , rem );
        __m128i out = _load_xmm( accu_c , rem );
        __m128i r0 = linear_transform_8x8_128b( ml , mh , inp , mask );
        r0 ^= out;
        _store_xmm( accu_c , rem , r0 );
    }
}

/////////////////////////////////////////////////////////////////////////////////


// input a:          0x12 0x34 0x56 0x78 ......
// output x_align:   0x02 0x01 0x04 0x03 0x06 0x05 .........
static inline
void gf16v_split_16to32_sse( __m128i * x_align , __m128i a )
{
    __m128i mask_f = _mm_set1_epi8(0xf);
    __m128i al = a&mask_f;
    __m128i ah = _mm_srli_epi16( a,4 )&mask_f;

    __m128i a0 = _mm_unpacklo_epi8( al , ah );
    __m128i a1 = _mm_unpackhi_epi8( al , ah );

    _mm_store_si128( x_align , a0 );
    _mm_store_si128( x_align + 1 , a1 );
}

typedef struct __xmm_x2 { __m128i v0; __m128i v1; } xmmx2_t;
static inline
xmmx2_t gf16v_split_16to32_sse2( __m128i a )
{
    __m128i mask_f = _mm_set1_epi8(0xf);
    __m128i al = a&mask_f;
    __m128i ah = _mm_srli_epi16( a,4 )&mask_f;
    xmmx2_t r;
    r.v0 = _mm_unpacklo_epi8( al , ah );
    r.v1 = _mm_unpackhi_epi8( al , ah );
    return r;
}
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//////////////////////////256-bits functions///////////////////////////////

//////////////////////////////from blas_sse.c///////////////////////////////
static inline
void gf256v_add_sse( uint8_t * accu_b, const uint8_t * a , unsigned _num_byte ) {
    while( _num_byte >= 16 ) {
        _mm_storeu_si128( (__m128i*) (accu_b) , _mm_loadu_si128((__m128i*)(a))^_mm_loadu_si128((__m128i*)(accu_b)) );
        a += 16;
        accu_b += 16;
        _num_byte -= 16;
    }
    for(unsigned j=0;j<_num_byte;j++) { accu_b[j] ^= a[j]; }
}
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////from gf16_avx2.h/////////////////////////////////
// 6 instructions
static inline __m256i linear_transform_8x8_256b( __m256i tab_l , __m256i tab_h , __m256i v , __m256i mask_f )
{
    return _mm256_shuffle_epi8(tab_l,v&mask_f)^_mm256_shuffle_epi8(tab_h,_mm256_srli_epi16(v,4)&mask_f);
}

//
// generate multiplication table for '4-bit' variable 'b'
//
static inline __m256i tbl32_gf16_multab( uint8_t b )
{
#if 1
    __m256i bx = _mm256_set1_epi16( b&0xf );
    __m256i b1 = _mm256_srli_epi16( bx , 1 );

    __m256i tab0 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*0));
    __m256i tab1 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*1));
    __m256i tab2 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*2));
    __m256i tab3 = _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*3));

    __m256i mask_1  = _mm256_set1_epi16(1);
    __m256i mask_4  = _mm256_set1_epi16(4);
    __m256i mask_0  = _mm256_setzero_si256();

    return ( tab0 & _mm256_cmpgt_epi16( bx&mask_1  , mask_0) )
           ^ ( tab1 & _mm256_cmpgt_epi16( b1&mask_1  , mask_0) )
           ^ ( tab2 & _mm256_cmpgt_epi16( bx&mask_4  , mask_0) )
           ^ ( tab3 & _mm256_cmpgt_epi16( b1&mask_4  , mask_0) );
#else
    __m256i bx1 = _mm256_set1_epi8( b&0xf );
    __m256i mask_1 = _mm256_set1_epi8(1);
    __m256i mask_2 = _mm256_set1_epi8(2);
    __m256i mask_4 = _mm256_set1_epi8(4);
    __m256i mask_8 = _mm256_set1_epi8(8);

    return (_mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*0)) &_mm256_cmpeq_epi8(mask_1,bx1&mask_1))
		^ ( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*1)) & _mm256_cmpeq_epi8(mask_2,bx1&mask_2) )
		^ ( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*2)) & _mm256_cmpeq_epi8(mask_4,bx1&mask_4) )
		^ ( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*3)) & _mm256_cmpeq_epi8(mask_8,bx1&mask_8) );
#endif
}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////from blas_avx2.c///////////////////////////////

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////from blas_avx2.h////////////////////////////////
static inline
void gf256v_add_avx2( uint8_t * accu_b, const uint8_t * a , unsigned _num_byte ) {
    while( _num_byte >= 32 ) {
        _mm256_storeu_si256( (__m256i*)accu_b, _mm256_loadu_si256((__m256i*)a) ^ _mm256_loadu_si256((__m256i*)accu_b) );
        accu_b += 32;
        a += 32;
        _num_byte -= 32;
    }
    if(_num_byte) gf256v_add_sse( accu_b , a , _num_byte );
}

static inline
__m256i _load_ymm( const uint8_t *a , unsigned _num_byte ) {
    uint8_t temp[32] __attribute__((aligned(32)));
    //assert( 32 >= _num_byte );
    //assert( 0 < _num_byte );
    for(unsigned i=0;i<_num_byte;i++) temp[i] = a[i];
    return _mm256_load_si256((__m256i*)temp);
}

static inline
void loadu_ymm( __m256i *ymm_a, const uint8_t *a, unsigned _num_byte ) {
    unsigned n_32 = (_num_byte>>5);
    unsigned n_32_rem = _num_byte&0x1f;
    while( n_32-- ) {
        ymm_a[0] = _mm256_loadu_si256( (__m256i*)(a) );
        ymm_a++;
        a += 32;
    }
    if( n_32_rem ) ymm_a[0] = _load_ymm( a , n_32_rem );
}



static inline
void _store_ymm( uint8_t *a , unsigned _num_byte , __m256i data ) {
    uint8_t temp[32] __attribute__((aligned(32)));
    //assert( 32 >= _num_byte );
    //assert( 0 < _num_byte );
    _mm256_store_si256((__m256i*)temp,data);
    for(unsigned i=0;i<_num_byte;i++) a[i] = temp[i];
}


static inline
void storeu_ymm( uint8_t *a , unsigned _num_byte , __m256i *ymm_a ) {
    unsigned n_32 = (_num_byte>>5);
    unsigned n_32_rem = _num_byte&0x1f;
    while( n_32-- ) {
        _mm256_storeu_si256( (__m256i*)a , ymm_a[0] );
        ymm_a++;
        a += 32;
    }
    if( n_32_rem ) _store_ymm( a , n_32_rem , ymm_a[0] );
}

static inline
void linearmap_8x8_ymm( uint8_t * a , __m256i ml , __m256i mh , __m256i mask , unsigned _num_byte ) {
    unsigned n_32 = _num_byte>>5;
    unsigned rem = _num_byte&31;
    while(n_32--){
        __m256i inp = _mm256_loadu_si256( (__m256i*)a );
        __m256i r0 = linear_transform_8x8_256b( ml , mh , inp , mask );
        _mm256_storeu_si256( (__m256i*)a , r0 );
        a += 32;
    }
    if( rem ) linearmap_8x8_sse( a , _mm256_castsi256_si128(ml) , _mm256_castsi256_si128(mh) , _mm256_castsi256_si128(mask) , rem );
}

///
/// \param accu_c
/// \param a
/// \param ml
/// \param mh
/// \param mask
/// \param _num_byte
static inline
void linearmap_8x8_accu_ymm( uint8_t * accu_c , const uint8_t * a ,  __m256i ml , __m256i mh , __m256i mask , unsigned _num_byte ) {
    unsigned n_32 = _num_byte>>5;
    unsigned rem = _num_byte&31;
    while(n_32--){
        __m256i inp = _mm256_loadu_si256( (__m256i*)a );
        __m256i out = _mm256_loadu_si256( (__m256i*)accu_c );
        __m256i r0 = out ^ linear_transform_8x8_256b( ml , mh , inp , mask );
        _mm256_storeu_si256( (__m256i*)accu_c , r0 );
        a += 32;
        accu_c += 32;
    }
    if( rem )
        linearmap_8x8_accu_sse( accu_c , a , _mm256_castsi256_si128(ml) , _mm256_castsi256_si128(mh) , _mm256_castsi256_si128(mask) , rem );
}

static inline
void gf16v_mul_scalar_avx2( uint8_t * a, uint8_t gf16_b , unsigned _num_byte ) {
    __m256i ml = tbl32_gf16_multab( gf16_b );
    __m256i mh = _mm256_slli_epi16( ml , 4 );
    __m256i mask = _mm256_set1_epi8(0xf);
    linearmap_8x8_ymm( a , ml , mh , mask , _num_byte );
}


///
/// \param accu_c
/// \param a
/// \param gf16_b
/// \param _num_byte
static inline
void gf16v_madd_avx2( uint8_t * accu_c, const uint8_t * a , uint8_t gf16_b, unsigned _num_byte ) {
    __m256i ml = tbl32_gf16_multab( gf16_b );
    __m256i mh = _mm256_slli_epi16( ml , 4 );
    __m256i mask = _mm256_set1_epi8(0xf);
    linearmap_8x8_accu_ymm( accu_c , a , ml , mh , mask , _num_byte );
}

void gf16v_madd_avx2_(uint8_t * accu_c, const uint8_t * a , uint8_t gf16_b, unsigned _num_byte ) {
    const __m256i B = _mm256_set1_epi8(gf16_b);
    const uint32_t nrA256 = (_num_byte+31) >> 5;
    const uint32_t n = _num_byte%32 == 0 ? 8 : ((_num_byte%32)+3)/4;
    const __m256i loadMask = _mm256_setr_epi32(-1,(n>1)?(-1):0,(n>2)?(-1):0,(n>3)?(-1):0,(n>4)?(-1):0,(n>5)?(-1):0,(n>6)?(-1):0,(n>7)?(-1):0);

    for (uint32_t i = 0; i < nrA256-1; i++) {
        const __m256i A = _mm256_loadu_si256((__m256i *)(a + 32*i));
        __m256i C = _mm256_loadu_si256((__m256i *)(accu_c + 32*i));
        C ^= gf16_mult_avx_compressed_2(A, B);

        _mm256_storeu_si256((__m256i *)(accu_c + 32*i), C);
    }

    // tail mng
    const __m256i A = _mm256_maskload_epi32((int *)(a + (nrA256 - 1)*32), loadMask);
    __m256i C = _mm256_loadu_si256((__m256i *)(accu_c + 32*(nrA256-1)));
    C ^= gf16_mult_avx_compressed_2(A, B);

    _mm256_maskstore_epi32((int *)(accu_c + 32*(nrA256-1)), loadMask, C);
}

static inline
void gf16v_madd_multab_avx2( uint8_t * accu_c, const uint8_t * a , const uint8_t * multab , unsigned _num_byte ) {
    __m256i ml = _mm256_load_si256( (__m256i*) (multab) );
    __m256i mh = _mm256_slli_epi16( ml , 4 );
    __m256i mask = _mm256_set1_epi8(0xf);

    linearmap_8x8_accu_ymm( accu_c , a , ml , mh , mask , _num_byte );
}
////////////////////   Generating multiplication tables  /////////////////////////////////////
//
//
////
//// Caution: multabs are different from ssse3 version
//// ssse3:  [multab_low] [ multab_high ]
////         <-   16  ->  <-    16     ->
//// avx2:   [         multab_low       ]
////         <---        32          --->
static inline
void gf16v_generate_multab_16_avx2( __m256i *multabs , __m128i a , unsigned len )
{
    __m256i broadcast_x1 = _mm256_set_epi8( 0,-16,0,-16, 0,-16,0,-16,  0,-16,0,-16, 0,-16,0,-16,  0,-16,0,-16, 0,-16,0,-16,  0,-16,0,-16, 0,-16,0,-16 );
    __m256i broadcast_x2 = _mm256_set_epi8( 0,0,-16,-16, 0,0,-16,-16,  0,0,-16,-16, 0,0,-16,-16,  0,0,-16,-16, 0,0,-16,-16,  0,0,-16,-16, 0,0,-16,-16 );
    __m256i broadcast_x4 = _mm256_set_epi8( 0,0,0,0, -16,-16,-16,-16,  0,0,0,0, -16,-16,-16,-16,  0,0,0,0, -16,-16,-16,-16,  0,0,0,0, -16,-16,-16,-16 );
    __m256i broadcast_x8 = _mm256_set_epi8( 0,0,0,0, 0,0,0,0,  -16,-16,-16,-16, -16,-16,-16,-16,  0,0,0,0, 0,0,0,0,  -16,-16,-16,-16, -16,-16,-16,-16 );

    //__m256i bx1 = _mm256_inserti128_si256( _mm256_castsi128_si256(a), a , 1 );
    __m256i bx1 = _mm256_setr_m128i( a , a );
    __m256i bx2 = _mm256_shuffle_epi8( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32 ))   , bx1 );
    __m256i bx4 = _mm256_shuffle_epi8( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*2 )) , bx1 );
    __m256i bx8 = _mm256_shuffle_epi8( _mm256_load_si256((__m256i const *) (__gf16_mulbase + 32*3 )) , bx1 );


    multabs[0] =  _mm256_shuffle_epi8(bx1,broadcast_x1) ^ _mm256_shuffle_epi8(bx2,broadcast_x2)
                  ^ _mm256_shuffle_epi8(bx4,broadcast_x4) ^ _mm256_shuffle_epi8(bx8,broadcast_x8);
    for(unsigned i=1;i<len;i++) {
        bx1 = _mm256_srli_si256( bx1 , 1 );
        bx2 = _mm256_srli_si256( bx2 , 1 );
        bx4 = _mm256_srli_si256( bx4 , 1 );
        bx8 = _mm256_srli_si256( bx8 , 1 );

        multabs[i] =  _mm256_shuffle_epi8(bx1,broadcast_x1) ^ _mm256_shuffle_epi8(bx2,broadcast_x2)
                      ^ _mm256_shuffle_epi8(bx4,broadcast_x4) ^ _mm256_shuffle_epi8(bx8,broadcast_x8);
    }
}

static inline
void _gf16v_generate_multabs_avx2( __m256i * multabs, const uint8_t * v , unsigned n_ele )
{
    __m128i x[2];
    while(n_ele>=32){
        x[0] = _mm_loadu_si128( (__m128i*)v );
        gf16v_split_16to32_sse( x , x[0] );
        gf16v_generate_multab_16_avx2( multabs , x[0] , 16 );
        gf16v_generate_multab_16_avx2( multabs+16 , x[1] , 16 );
        multabs += 32;
        v += 16;
        n_ele -= 32;
    }
    if(n_ele){
        x[0] = _load_xmm( v , (n_ele+1)>>1 );
        gf16v_split_16to32_sse( x , x[0] );
        if(n_ele<=16) {
            gf16v_generate_multab_16_avx2( multabs , x[0] , n_ele );
        } else {
            gf16v_generate_multab_16_avx2( multabs , x[0] , 16 );
            gf16v_generate_multab_16_avx2( multabs+16 , x[1] , n_ele-16 );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

/////////////////////////////from blas_common.c//////////////////////////////
#define gf256v_add          gf256v_add_avx2
void gf256v_set_zero(uint8_t *b, unsigned _num_byte)
{
    gf256v_add(b, b, _num_byte);
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////from blas_matrix_avx2.c/////////////////////////
static inline
void gf16mat_blockmat_madd_avx2( __m256i * dest , const uint8_t * org_mat , unsigned mat_vec_byte , unsigned blk_st_idx , unsigned blk_vec_byte ,
                                 const __m256i * multab_vec_ele , unsigned n_vec_ele )
{
    unsigned n_full_ymm = blk_vec_byte >> 5;
    unsigned n_rem_byte = blk_vec_byte & 31;
    __m256i mask_f = _mm256_set1_epi8(0xf);

    org_mat += blk_st_idx;
    if (!n_rem_byte) {
        for(unsigned i = 0; i < n_vec_ele; i++ ) {
            __m256i tab_l = multab_vec_ele[0];
            __m256i tab_h = _mm256_slli_epi16(tab_l,4);;
            multab_vec_ele ++;

            for(unsigned j=0;j<n_full_ymm;j++) {
                __m256i mj = _mm256_loadu_si256( (__m256i*)(org_mat+j*32) );
                dest[j] ^= linear_transform_8x8_256b( tab_l , tab_h , mj , mask_f );
            }
            org_mat += mat_vec_byte;
        }
    } else {
        for(unsigned i = 0; i < n_vec_ele-1; i++ ) {
            __m256i tab_l = multab_vec_ele[0];
            __m256i tab_h = _mm256_slli_epi16(tab_l,4);;
            multab_vec_ele ++;
            for(unsigned j=0;j<n_full_ymm+1;j++) {
                __m256i mj = _mm256_loadu_si256( (__m256i*)(org_mat+j*32) );
                dest[j] ^= linear_transform_8x8_256b( tab_l , tab_h , mj , mask_f );
            }
            org_mat += mat_vec_byte;
        }{ // i = n_vec_ele-1
            __m256i tab_l = multab_vec_ele[0];
            __m256i tab_h = _mm256_slli_epi16(tab_l,4);;
            multab_vec_ele ++;
            for(unsigned j=0;j<n_full_ymm;j++) {
                __m256i mj = _mm256_loadu_si256( (__m256i*)(org_mat+j*32) );
                dest[j] ^= linear_transform_8x8_256b( tab_l , tab_h , mj , mask_f );
            } //if( n_rem_byte )
            { // j = n_full_ymm;
                __m256i mj = _load_ymm( org_mat+(n_full_ymm*32) , n_rem_byte );
                dest[n_full_ymm] ^= linear_transform_8x8_256b( tab_l , tab_h , mj , mask_f );
            }
        }
    }
}

static void gf16mat_prod_multab_16x16_avx2( uint8_t * c , const uint8_t * matA , const __m256i * multab_b )
{
    __m256i mask_f = _mm256_set1_epi8(0xf);

    __m256i ma0123 = _mm256_loadu_si256((const __m256i*) (matA) );    // 4 rows
    __m256i ma4567 = _mm256_loadu_si256((const __m256i*) (matA+32) ); // 4 rows
    __m256i mb02 = _mm256_permute2x128_si256( multab_b[0] , multab_b[2] , 0x20 );
    __m256i mb13 = _mm256_permute2x128_si256( multab_b[1] , multab_b[3] , 0x20 );
    __m256i mb46 = _mm256_permute2x128_si256( multab_b[4] , multab_b[6] , 0x20 );
    __m256i mb57 = _mm256_permute2x128_si256( multab_b[5] , multab_b[7] , 0x20 );
    __m256i ma0123l = ma0123&mask_f;
    __m256i ma0123h = _mm256_srli_epi16(ma0123,4)&mask_f;
    __m256i ma4567l = ma4567&mask_f;
    __m256i ma4567h = _mm256_srli_epi16(ma4567,4)&mask_f;
    __m256i ma02 = _mm256_unpacklo_epi64(ma0123l,ma0123h);
    __m256i ma13 = _mm256_unpackhi_epi64(ma0123l,ma0123h);
    __m256i ma46 = _mm256_unpacklo_epi64(ma4567l,ma4567h);
    __m256i ma57 = _mm256_unpackhi_epi64(ma4567l,ma4567h);
    __m256i r = _mm256_shuffle_epi8(mb02,ma02)^_mm256_shuffle_epi8(mb13,ma13)^_mm256_shuffle_epi8(mb46,ma46)^_mm256_shuffle_epi8(mb57,ma57);

    __m256i ma89ab = _mm256_loadu_si256((const __m256i*) (matA+64) );    // 4 rows
    __m256i macdef = _mm256_loadu_si256((const __m256i*) (matA+96) ); // 4 rows
    __m256i mb8a = _mm256_permute2x128_si256( multab_b[8] , multab_b[10] , 0x20 );
    __m256i mb9b = _mm256_permute2x128_si256( multab_b[9] , multab_b[11] , 0x20 );
    __m256i mbce = _mm256_permute2x128_si256( multab_b[12] , multab_b[14] , 0x20 );
    __m256i mbdf = _mm256_permute2x128_si256( multab_b[13] , multab_b[15] , 0x20 );
    __m256i ma89abl = ma89ab&mask_f;
    __m256i ma89abh = _mm256_srli_epi16(ma89ab,4)&mask_f;
    __m256i macdefl = macdef&mask_f;
    __m256i macdefh = _mm256_srli_epi16(macdef,4)&mask_f;
    __m256i ma8a = _mm256_unpacklo_epi64(ma89abl,ma89abh);
    __m256i ma9b = _mm256_unpackhi_epi64(ma89abl,ma89abh);
    __m256i mace = _mm256_unpacklo_epi64(macdefl,macdefh);
    __m256i madf = _mm256_unpackhi_epi64(macdefl,macdefh);
    r ^= _mm256_shuffle_epi8(mb8a,ma8a)^_mm256_shuffle_epi8(mb9b,ma9b)^_mm256_shuffle_epi8(mbce,mace)^_mm256_shuffle_epi8(mbdf,madf);

    __m128i rr = _mm256_castsi256_si128( r ) ^ _mm256_extractf128_si256( r , 1 );
    __m128i rr2 = rr ^ _mm_srli_si128( _mm_slli_epi16(rr,4) , 8 );

    uint8_t temp[16] __attribute__((aligned(16)));
    _mm_store_si128( (__m128i*)temp , rr2 );
    for(int i=0;i<8;i++) c[i] = temp[i];
}

static void gf16mat_prod_16x16_avx2( uint8_t * c , const uint8_t * matA , const uint8_t * b )
{
    __m256i multabs[16];
    uint8_t temp[16] __attribute__((aligned(16)));
    for(int i=0;i<8;i++) temp[i]=b[i];
    __m128i x0 = _mm_load_si128((const __m128i*)temp);
    xmmx2_t xx = gf16v_split_16to32_sse2( x0 );
    gf16v_generate_multab_16_avx2( multabs    , xx.v0 , 16 );

    gf16mat_prod_multab_16x16_avx2( c , matA , multabs );
}

void gf16mat_prod_avx2( uint8_t * c , const uint8_t * matA , unsigned matA_vec_byte , unsigned matA_n_vec , const uint8_t * b )
{
//    if((32==matA_vec_byte)) { gf16mat_prod_64x_avx2(c,matA,matA_n_vec,b); return; }
//    if((16==matA_vec_byte)&&(32==matA_n_vec)) { gf16mat_prod_32x32_avx2(c,matA,b); return; }
    if((8==matA_vec_byte) &&(16==matA_n_vec)) { gf16mat_prod_16x16_avx2(c,matA,b); return; }

    __m256i multabs[32];
    gf256v_set_zero( c , matA_vec_byte );

    __m256i blockmat_vec[8];

    while(matA_n_vec) {
        unsigned n_ele = (32<=matA_n_vec)? 32: matA_n_vec;
        _gf16v_generate_multabs_avx2( multabs , b , n_ele );

        unsigned vec_len_to_go = matA_vec_byte;
        while( vec_len_to_go ) {
            unsigned block_len = (vec_len_to_go >= 8*32)? 8*32 : vec_len_to_go;
            unsigned block_st_idx = matA_vec_byte - vec_len_to_go;

            loadu_ymm( blockmat_vec , c + block_st_idx , block_len );
            gf16mat_blockmat_madd_avx2( blockmat_vec , matA , matA_vec_byte , block_st_idx , block_len , multabs , n_ele );
            storeu_ymm( c + block_st_idx , block_len , blockmat_vec );

            vec_len_to_go -= block_len;
        }

        matA_n_vec -= n_ele;
        b += (n_ele>>1);
        matA += n_ele*matA_vec_byte;
    }
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////from blas_matrix.c///////////////////////////////
#define gf16mat_prod_impl             gf16mat_prod_avx2
void gf16mat_prod(uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b)
{
    gf16mat_prod_impl( c, matA, n_A_vec_byte, n_A_width, b);
}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////from blas_matrix_ref.c///////////////////////////
/////////////////////////  matrix-matrix multiplication  //////////////////////


void gf16mat_rowmat_mul_ref(uint8_t *matC, const uint8_t *matA, unsigned height_A, unsigned width_A_byte, const uint8_t *matB, unsigned width_B_byte)
{
    // NOTE: very important now
    gf256v_set_zero(matC , height_A*width_B_byte);
    gf16mat_new_core(matC, matB, height_A, width_B_byte, 2*width_A_byte, matA);
    return;

    //for( unsigned i=0; i<height_A; i++) {
    //    gf16mat_prod(matC, matB , width_B_byte , width_A_byte*2 , matA );
    //    matC += width_B_byte;
    //    matA += width_A_byte;
    //}
}



void gf16mat_colmat_mul_ref(uint8_t *mat_c, const uint8_t *mat_a, unsigned a_veclen_byte, unsigned a_n_vec, const uint8_t *mat_b, unsigned b_n_vec)
{
    gf16mat_rowmat_mul_ref( mat_c , mat_b , b_n_vec , (a_n_vec+1)>>1 , mat_a , a_veclen_byte );
}

///////////////////////////////////////////////////////////////////////////////

#define gf16mat_rowmat_mul_impl             gf16mat_rowmat_mul_ref
//////////////////////////////from blas_matrix.c ///////////////////////////
/// @brief matrix multiplication:  matC = matA * matB , in GF(16)
///
/// @param[out]  matC         - the output row-major matrix C
/// @param[in]   matA         - a row-major matrix A.
/// @param[in]   height_A     - the number of row vectors in the matrix A.
/// @param[in]   width_A_byte  - the size of row vectors of A in bytes.
/// @param[in]   matB            - a row-major matrix B.
/// @param[in]   width_B_byte  - the size of row vectors of B in bytes.
///
/// \param matC
/// \param matA
/// \param height_A
/// \param width_A_byte
/// \param matB
/// \param width_B_byte
void gf16mat_rowmat_mul(uint8_t *matC, const uint8_t *matA, unsigned height_A, unsigned width_A_byte, const uint8_t *matB, unsigned width_B_byte)
{
    gf16mat_rowmat_mul_impl( matC, matA, height_A, width_A_byte, matB, width_B_byte);
}

#define gf16mat_colmat_mul_impl             gf16mat_colmat_mul_ref
/// @brief (column-major) matrix multiplication:  matC = matA * matB , in GF(16)
///
/// @param[out]  matC           - the output column-major matrix C
/// @param[in]   mat_a          - a column-major matrix A.
/// @param[in]   a_veclen_byte  - the vector length (height) of the matrix A in byte.
/// @param[in]   a_n_vec        - the number of vectors (width) in the matrix A.
/// @param[in]   mat_b          - a column-major matrix B.
/// @param[in]   b_n_vec        - the number of vectors in the matrix B.
///
void gf16mat_colmat_mul(uint8_t *mat_c, const uint8_t *mat_a, unsigned a_veclen_byte, unsigned a_n_vec, const uint8_t *mat_b, unsigned b_n_vec)
{
    gf16mat_colmat_mul_impl( mat_c , mat_a , a_veclen_byte , a_n_vec , mat_b , b_n_vec );
}
///////////////////////////////////////////////////////////////////////////////



#endif /* MIRITH_DSS_BLAS_AVX2_H */
