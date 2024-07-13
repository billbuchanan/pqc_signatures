/* Based on the public domain implementation in pqov-paper/src/neon/ from
 * https://github.com/pqov/pqov-paper by Ward Beullens, Ming-Shing Chen,
 * Shih-Hao Hung, Matthias J. Kannwischer, Bo-Yuan Peng, Cheng-Jhih Shih,
 * and Bo-Yin Yang */

#ifndef MIRITH_DSS_BLAS_AVX2_H
#define MIRITH_DSS_BLAS_AVX2_H
/// @file blas_neon.h
/// @brief Inlined functions for implementing basic linear algebra functions for AVX2 arch.
///

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <arm_neon.h>
#include "matrix_constants.h"


static inline
uint8x16_t _load_Qreg( const uint8_t * v , unsigned n ) {
    uint8_t temp[16];
    //n &= 15;
    for(unsigned i=0;i<n;i++) temp[i]=v[i];
    return vld1q_u8( temp );
}

static inline
void load_Qregs( uint8x16_t *r, const uint8_t * v , unsigned n ) {
    while( n>=16 ) {
        r[0] = vld1q_u8( v );
        r++;
        v+=16;
        n-=16;
    }
    if(n) r[0] = _load_Qreg(v,n);
}

static inline
void _store_Qreg( uint8_t * v , unsigned n , uint8x16_t a ) {
    uint8_t temp[16];
    //n &= 15;
    vst1q_u8( temp , a );
    for(unsigned i=0;i<n;i++) v[i] = temp[i];
}

static inline
void store_Qregs( uint8_t * v , unsigned n , const uint8x16_t *r ) {
    while( n>=16 ) {
        vst1q_u8( v , r[0] );
        r++;
        v+=16;
        n-=16;
    }
    if(n) _store_Qreg(v,n,r[0]);
}

static inline
uint8_t gf16_inv_neon(uint8_t a)
{
    uint8x16_t tinv = vld1q_u8( __gf16_inv );
    uint8x8_t aa  = vdup_n_u8( a );
    uint8x8_t r   = vqtbl1_u8( tinv , aa );
    return vget_lane_u8(r,0);
}

// gf16 := gf2[x]/(x^4+x+1)
static inline
uint8x16_t _gf16v_reduce_tbl_neon( uint8x16_t abl , uint8x16_t abh , uint8x16_t tab_reduce )
{
    poly8x16_t rl = abl ^ vqtbl1q_u8( tab_reduce , vshrq_n_u8(abl,4) );
    poly8x16_t rh = abh ^ vqtbl1q_u8( tab_reduce , vshrq_n_u8(abh,4) );

    return vsliq_n_u8( rl , rh , 4 );
}

// gf16 := gf2[x]/(x^4+x+1)
static inline
uint8x16_t _gf16v_reduce_pmul_neon( uint8x16_t abl , uint8x16_t abh , uint8x16_t mask_3 )
{
    poly8x16_t rl = abl ^ vmulq_p8( vshrq_n_u8(abl,4) , mask_3 );
    poly8x16_t rh = abh ^ vmulq_p8( vshrq_n_u8(abh,4) , mask_3 );

    return vsliq_n_u8( rl , rh , 4 );
}

// Favor VTBL more than PMUL:
// For cortex-A72, VTBL  (Latency:3,Throughput:2) is faster than PMUL(L:5,T:0.5)
//
#define _GF16_REDUCE_WITH_TBL_

static inline
uint8x16_t _gf16v_mul_neon( uint8x16_t a , uint8x16_t bp , uint8x16_t mask_f , uint8x16_t mask_3 )
{
    uint8x16_t al0 = a&mask_f;
    uint8x16_t ah0 = vshrq_n_u8( a , 4 );
// mul
    poly8x16_t abl = vmulq_p8( al0 , bp );
    poly8x16_t abh = vmulq_p8( ah0 , bp );
// reduce
#if defined(_GF16_REDUCE_WITH_TBL_)
    return _gf16v_reduce_tbl_neon( abl , abh , mask_3 );
#else
    return _gf16v_reduce_pmul_neon( abl , abh , mask_3 );
#endif
}

static inline
uint8x16_t gf16v_mul_neon( uint8x16_t a , uint8_t b )
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t bp = vdupq_n_u8(b);
#if defined( _GF16_REDUCE_WITH_TBL_)
    uint8x16_t tab_reduce = vld1q_u8(__gf16_reduce);
#else
    //uint8x16_t mask_3 = vdupq_n_u8( 3 );
    uint8x16_t tab_reduce = vdupq_n_u8( 3 );
#endif
    return _gf16v_mul_neon( a , bp , mask_f , tab_reduce );
}

static inline
uint8x16_t gf16v_mul_neon2( uint8x16_t a , uint8x16_t b )
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
#if defined( _GF16_REDUCE_WITH_TBL_)
    uint8x16_t tab_reduce = vld1q_u8(__gf16_reduce);
#else
    uint8x16_t tab_reduce = vdupq_n_u8( 3 );
#endif
    return _gf16v_mul_neon( a , b , mask_f , tab_reduce );
}

static inline
uint8x16_t _gf16v_mul_unpack_neon( uint8x16_t a0 , uint8x16_t b0 , uint8x16_t tab_reduce )
{
    uint8x16_t ab = vreinterpretq_u8_p8(vmulq_p8( a0 , b0 ));
    return ab^vqtbl1q_u8( tab_reduce , vshrq_n_u8(ab,4) );
}

static inline
uint8x16_t _gf16v_get_multab_neon( uint8x16_t b , uint8x16_t tab_reduce , uint8x16_t tab_0_f ) { return _gf16v_mul_unpack_neon(b,tab_0_f,tab_reduce); }

static inline
uint8x16_t gf16v_get_multab_neon( uint8_t b )
{
    uint8x16_t tab_reduce = vld1q_u8(__gf16_reduce);
    uint8x16_t tab_0_f = vld1q_u8(__0_f);
    uint8x16_t bb = vdupq_n_u8(b);
    return _gf16v_get_multab_neon(bb,tab_reduce,tab_0_f);
}

static inline
uint8x16_t _gf16_tbl_x2( uint8x16_t a , uint8x16_t tbl , uint8x16_t mask_f ) {
    return vsliq_n_u8( vqtbl1q_u8( tbl , a&mask_f ) , vqtbl1q_u8( tbl , vshrq_n_u8( a , 4 ) ), 4 );
}


// some information
void gf16mat_new_core(uint8_t *__restrict c,
                      const uint8_t *__restrict__ a,
                      const unsigned int nr_cols_B,
                      const unsigned int column_A_bytes,
                      const unsigned int nr_cols_A,
                      const uint8_t *__restrict__ b) {
    //assert(column_A_bytes <= 16);
    //assert(nr_cols_A <= 32);
    const uint32_t nr_bytes_B_col = nr_cols_A >> 1;
    const uint32_t maxByteInRegister 	= (16 / column_A_bytes) * column_A_bytes;
    const uint32_t colsInRegister 		= (16 / column_A_bytes);
    const uint32_t nrA                  = ((column_A_bytes*nr_cols_A)+maxByteInRegister-1) / maxByteInRegister; 
    __uint128_t ret = 0;
	
	uint8x16_t Adata[nrA];
    uint8_t bmasked[256] __attribute__((aligned(32))) = {0};

    for (uint32_t i = 0; i < nrA; ++i) {
        Adata[i] = vld1q_u8(a + i*maxByteInRegister);
    }

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

        uint8x16_t tmp = {0};
        for (uint32_t i = 0; i < nrA; ++i) {
            uint8x16_t Bdata = vld1q_u8(bmasked + i*maxByteInRegister);
            tmp ^= gf16v_mul_neon2(Adata[i], Bdata);
        }

        vst1q_u8(bmasked, tmp);
        ret = 0;
        for (uint32_t i = 0; i < colsInRegister; i++) {
            ret ^= (*((__uint128_t *)(bmasked + i*column_A_bytes)));
        }

        for (uint32_t i = 0; i < column_A_bytes; i++) {
            c[k*column_A_bytes + i] ^= (uint8_t)(ret>>(i*8));
        }
    }
}


static inline
void gf16v_mul_scalar_neon( uint8_t * a, uint8_t gf16_b , unsigned _num_byte ) {
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t bp = vdupq_n_u8(gf16_b&0xf);
#if defined(_GF16_REDUCE_WITH_TBL_)
    uint8x16_t mask_3 = vld1q_u8(__gf16_reduce);
#else
    uint8x16_t mask_3 = vdupq_n_u8( 3 );
#endif

    while( _num_byte >= 16 ) {
        uint8x16_t aa = vld1q_u8(a);
        vst1q_u8( a , _gf16v_mul_neon(aa,bp,mask_f,mask_3) );
        _num_byte -= 16;
        a += 16;
    }
    if(_num_byte) {
        uint8_t temp[16];
        for(unsigned j=0;j<_num_byte;j++) temp[j]=a[j];
        uint8x16_t aa = vld1q_u8(temp);
        vst1q_u8( temp , _gf16v_mul_neon(aa,bp,mask_f,mask_3) );
        for(unsigned j=0;j<_num_byte;j++) a[j]=temp[j];
    }
}



static inline
void gf16v_madd_neon( uint8_t * accu_c, const uint8_t * a , uint8_t gf16_b, unsigned _num_byte ) {
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t bp = vdupq_n_u8(gf16_b&0xf);
#if defined(_GF16_REDUCE_WITH_TBL_)
    uint8x16_t mask_3 = vld1q_u8(__gf16_reduce);
#else
    uint8x16_t mask_3 = vdupq_n_u8( 3 );
#endif
    while( _num_byte >= 16 ) {
        uint8x16_t aa = vld1q_u8(a);
        uint8x16_t cc = vld1q_u8(accu_c);
        vst1q_u8( accu_c , cc^_gf16v_mul_neon(aa,bp,mask_f,mask_3) );
        _num_byte -= 16;
        a += 16;
        accu_c += 16;
    }
    if(_num_byte) {
        uint8_t temp[16];
        for(unsigned j=0;j<_num_byte;j++) temp[j]=a[j];
        uint8x16_t aa = vld1q_u8(temp);
        vst1q_u8( temp , _gf16v_mul_neon(aa,bp,mask_f,mask_3) );
        for(unsigned j=0;j<_num_byte;j++) accu_c[j] ^= temp[j];
    }
}

static inline
void gf256v_add_neon( uint8_t * accu_b, const uint8_t * a , unsigned _num_byte ) {
    while( _num_byte >= 48 ) {
        uint8x16_t a0 = vld1q_u8(a);
        uint8x16_t a1 = vld1q_u8(a+16);
        uint8x16_t a2 = vld1q_u8(a+32);
        uint8x16_t b0 = vld1q_u8(accu_b);
        uint8x16_t b1 = vld1q_u8(accu_b+16);
        uint8x16_t b2 = vld1q_u8(accu_b+32);
        vst1q_u8( accu_b    , a0^b0 );
        vst1q_u8( accu_b+16 , a1^b1 );
        vst1q_u8( accu_b+32 , a2^b2 );
        _num_byte -= 48;
        a += 48;
        accu_b += 48;
    }
    while( _num_byte >= 16 ) {
        vst1q_u8( accu_b , vld1q_u8(a)^vld1q_u8(accu_b) );
        _num_byte -= 16;
        a += 16;
        accu_b += 16;
    }
    //for(unsigned j=0;j<_num_byte;j++) { accu_b[j] ^= a[j]; }
    while( _num_byte-- ) { *accu_b ^= *a; accu_b++; a++; }
}

/////////////////////////////from blas_common.c//////////////////////////////
#define gf256v_add          gf256v_add_neon
void gf256v_set_zero(uint8_t *b, unsigned _num_byte)
{
    gf256v_add(b, b, _num_byte);
}
//////////////////////////////////////////////////////////////////////////////

/////////////////////////  matrix-matrix multiplication  //////////////////////

//////////// specialized functions  /////////////////////

static inline
void gf16mat_prod_32x_multab_neon(uint8_t * c , const uint8_t * matA , unsigned width_A , const uint8_t * multabs)
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t cc = _gf16_tbl_x2( vld1q_u8(matA) , vld1q_u8(multabs) , mask_f );
    for(int i=((int)width_A)-1;i>0;i--) {
        matA += 16;
        multabs += 16;
        cc ^= _gf16_tbl_x2( vld1q_u8(matA) , vld1q_u8(multabs) , mask_f );
    }
    vst1q_u8(c,cc);
}

static inline
void gf16mat_prod_64x_multab_neon(uint8_t * c , const uint8_t * matA , unsigned width_A, const uint8_t * multabs)
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t tab = vld1q_u8(multabs);  multabs += 16;
    uint8x16_t cc0 = _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    uint8x16_t cc1 = _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    for(int i=((int)width_A)-1;i>0;i--) {
        tab = vld1q_u8(multabs);  multabs += 16;
        cc0 ^= _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
        cc1 ^= _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    }
    vst1q_u8(c,cc0);
    vst1q_u8(c+16,cc1);
}

static inline
void gf16mat_prod_96x_multab_neon(uint8_t * c , const uint8_t * matA , unsigned width_A, const uint8_t * multabs)
{
    uint8x16_t mask_f = vdupq_n_u8( 0xf );
    uint8x16_t tab = vld1q_u8(multabs);  multabs += 16;
    uint8x16_t cc0 = _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    uint8x16_t cc1 = _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    uint8x16_t cc2 = _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    for(int i=((int)width_A)-1;i>0;i--) {
        tab = vld1q_u8(multabs);  multabs += 16;
        cc0 ^= _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
        cc1 ^= _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
        cc2 ^= _gf16_tbl_x2( vld1q_u8(matA) , tab , mask_f ); matA += 16;
    }
    vst1q_u8(c,cc0);
    vst1q_u8(c+16,cc1);
    vst1q_u8(c+32,cc2);
}

#define BLOCK_LEN 4


static
void gf16mat_blockmat_prod_multab_neon( uint8_t * dest , const uint8_t * org_mat , unsigned mat_vec_byte , unsigned blk_vec_byte ,
                                        const uint8x16_t * multab_vec_ele , unsigned n_vec_ele )
{
    unsigned n_full_xmm = blk_vec_byte >> 4;
    unsigned n_rem_byte = blk_vec_byte & 15;
    uint8x16_t mask_f = vdupq_n_u8(0xf);

    uint8x16_t tmp[BLOCK_LEN];
    for(int i=0;i<BLOCK_LEN;i++) tmp[i] = vdupq_n_u8(0);

    if( !n_rem_byte ) {
        for(unsigned i = 0; i < n_vec_ele; i++ ) {
            uint8x16_t tab = multab_vec_ele[0];
            multab_vec_ele += 1;

            for(unsigned j=0;j<n_full_xmm;j++) {
                uint8x16_t mj = vld1q_u8( org_mat+j*16 );
                tmp[j] ^= _gf16_tbl_x2( mj , tab , mask_f );
            }
            org_mat += mat_vec_byte;
        }
        for(unsigned i=0;i<n_full_xmm;i++) vst1q_u8(dest+i*16,tmp[i]);
    } else { // n_rem_byte
        for(unsigned i = 0; i < n_vec_ele-1; i++ ) {
            uint8x16_t tab = multab_vec_ele[0];
            multab_vec_ele += 1;

            for(unsigned j=0;j<=n_full_xmm;j++) {  // note : <=
                uint8x16_t mj = vld1q_u8( org_mat+j*16 );
                tmp[j] ^= _gf16_tbl_x2( mj , tab , mask_f );
            }
            org_mat += mat_vec_byte;
        } { //unsigned i = n_vec_ele-1;
            uint8x16_t tab = multab_vec_ele[0];
            for(unsigned j=0;j<n_full_xmm;j++) {
                uint8x16_t mj = vld1q_u8( org_mat+j*16 );
                tmp[j] ^= _gf16_tbl_x2( mj , tab , mask_f );
            } {
                //unsigned j=n_full_xmm;
                uint8x16_t mj = _load_Qreg( org_mat+(n_full_xmm*16) , n_rem_byte );
                tmp[n_full_xmm] ^= _gf16_tbl_x2( mj , tab , mask_f );
            }
        }
        for(unsigned i=0;i<n_full_xmm+1;i++) vst1q_u8(dest+i*16,tmp[i]);
        _store_Qreg( dest+n_full_xmm , n_rem_byte , tmp[n_full_xmm] );
    }
}



void gf16mat_prod_multab_neon( uint8_t * c , const uint8_t * matA , unsigned matA_vec_byte , unsigned matA_n_vec , const uint8_t * multab_b )
{
    if( (32==matA_vec_byte) ) { gf16mat_prod_64x_multab_neon(c,matA,matA_n_vec,multab_b); return; }
    if( (16==matA_vec_byte) ) { gf16mat_prod_32x_multab_neon(c,matA,matA_n_vec,multab_b); return; }
    if( (48==matA_vec_byte) ) { gf16mat_prod_96x_multab_neon(c,matA,matA_n_vec,multab_b); return; }

    const uint8x16_t * multabs = (const uint8x16_t*)multab_b;
    while(matA_n_vec) {
        unsigned n_ele = matA_n_vec;
        unsigned vec_len_to_go = matA_vec_byte;
        while( vec_len_to_go ) {
            unsigned block_len = (vec_len_to_go >= BLOCK_LEN*16)? BLOCK_LEN*16 : vec_len_to_go;
            unsigned block_st_idx = matA_vec_byte - vec_len_to_go;
            gf16mat_blockmat_prod_multab_neon( c + block_st_idx , matA + block_st_idx , matA_vec_byte , block_len , multabs , n_ele );
            vec_len_to_go -= block_len;
        }
        matA_n_vec -= n_ele;
        matA += n_ele*matA_vec_byte;
        multabs += n_ele;
    }
}


#undef BLOCK_LEN

static inline
void gf16v_generate_multabs_neon( uint8_t * multabs, const uint8_t * v , unsigned n_ele )
{
    uint8x16_t tab_reduce = vld1q_u8(__gf16_reduce);
    uint8x16_t tab_0_f = vld1q_u8(__0_f);

    while( 2 <= n_ele ) {
        uint8x16_t b0 = vdupq_n_u8(v[0]&0xf);
        uint8x16_t b1 = vdupq_n_u8(v[0]>>4);
        uint8x16_t mm0 = _gf16v_get_multab_neon(b0,tab_reduce,tab_0_f);
        uint8x16_t mm1 = _gf16v_get_multab_neon(b1,tab_reduce,tab_0_f);
        vst1q_u8( multabs , mm0 ); multabs += 16;
        vst1q_u8( multabs , mm1 ); multabs += 16;
        n_ele -= 2;
        v++;
    }
    if( n_ele ) {
        uint8x16_t b0 = vdupq_n_u8(v[0]&0xf);
        uint8x16_t mm0 = _gf16v_get_multab_neon(b0,tab_reduce,tab_0_f);
        vst1q_u8( multabs , mm0 );
    }
}

//////////////////  end of multab   //////////////////////////////




static inline
void gf16mat_prod_32x_neon(uint8_t * c , const uint8_t * matA , unsigned width_A, const uint8_t * b) {
    uint8_t multabs[32*16];
    uint8x16_t r = vdupq_n_u8(0);
    uint8x16_t tmp;
    while( width_A >= 32 ) {
        gf16v_generate_multabs_neon( multabs , b , 32 );
        gf16mat_prod_32x_multab_neon( (uint8_t*)&tmp,matA,32,multabs);
        r ^= tmp;
        b += 16;
        width_A -= 32;
        matA += 16*32;
    }
    if( width_A ) {
        gf16v_generate_multabs_neon( multabs , b , width_A );
        gf16mat_prod_32x_multab_neon( (uint8_t*)&tmp,matA, width_A ,multabs);
        r ^= tmp;
    }
    vst1q_u8(c,r);
}

static inline
void gf16mat_prod_64x_neon(uint8_t * c , const uint8_t * matA , unsigned width_A, const uint8_t * b) {
    uint8_t multabs[64*16];
    uint8x16_t r[2];
    r[0] = vdupq_n_u8(0);
    r[1] = vdupq_n_u8(0);
    uint8x16_t tmp[2];
    while( width_A >= 64 ) {
        gf16v_generate_multabs_neon( multabs , b , 64 );
        gf16mat_prod_64x_multab_neon( (uint8_t*)tmp,matA,64,multabs);
        r[0] ^= tmp[0];
        r[1] ^= tmp[1];
        b += 32;
        width_A -= 64;
        matA += 16*64*2;
    }
    if( width_A ) {
        gf16v_generate_multabs_neon( multabs , b , width_A );
        gf16mat_prod_64x_multab_neon( (uint8_t*)tmp,matA, width_A ,multabs);
        r[0] ^= tmp[0];
        r[1] ^= tmp[1];
    }
    vst1q_u8(c,r[0]);
    vst1q_u8(c+16,r[1]);
}

static inline
void gf16mat_prod_96x_neon(uint8_t * c , const uint8_t * matA , unsigned width_A, const uint8_t * b) {
    uint8_t multabs[64*16];
    uint8x16_t r[3];
    r[0] = vdupq_n_u8(0);
    r[1] = vdupq_n_u8(0);
    r[2] = vdupq_n_u8(0);
    uint8x16_t tmp[3];
    while( width_A >= 64 ) {
        gf16v_generate_multabs_neon( multabs , b , 64 );
        gf16mat_prod_96x_multab_neon( (uint8_t*)tmp,matA,64,multabs);
        r[0] ^= tmp[0];
        r[1] ^= tmp[1];
        r[2] ^= tmp[2];
        b += 32;
        width_A -= 64;
        matA += 16*64*3;
    }
    if( width_A ) {
        gf16v_generate_multabs_neon( multabs , b , width_A );
        gf16mat_prod_96x_multab_neon( (uint8_t*)tmp,matA, width_A ,multabs);
        r[0] ^= tmp[0];
        r[1] ^= tmp[1];
        r[2] ^= tmp[2];
    }
    vst1q_u8(c,r[0]);
    vst1q_u8(c+16,r[1]);
    vst1q_u8(c+32,r[2]);
}


//////////// end of specialized functions  /////////////////////



#define BLOCK_LEN 4


void gf16mat_prod_neon( uint8_t * c , const uint8_t * matA , unsigned matA_vec_byte , unsigned matA_n_vec , const uint8_t * b )
{
    if( (32==matA_vec_byte) ) { gf16mat_prod_64x_neon(c,matA,matA_n_vec,b); return; }
    if( (16==matA_vec_byte) ) { gf16mat_prod_32x_neon(c,matA,matA_n_vec,b); return; }
    if( (48==matA_vec_byte) ) { gf16mat_prod_96x_neon(c,matA,matA_n_vec,b); return; }

    uint8_t multabs[16*64];
    gf256v_set_zero( c , matA_vec_byte );

    uint8_t blockmat_vec[BLOCK_LEN*16];
    while(matA_n_vec) {

        unsigned n_ele = (matA_n_vec>=64)? 64: matA_n_vec;
        gf16v_generate_multabs_neon( multabs , b , n_ele );

        unsigned vec_len_to_go = matA_vec_byte;
        while( vec_len_to_go ) {
            unsigned block_len = (vec_len_to_go >= BLOCK_LEN*16)? BLOCK_LEN*16 : vec_len_to_go;
            unsigned block_st_idx = matA_vec_byte - vec_len_to_go;

            gf16mat_blockmat_prod_multab_neon( blockmat_vec , matA+block_st_idx , matA_vec_byte , block_len , (uint8x16_t*)multabs , n_ele );
            gf256v_add_neon( c + block_st_idx , blockmat_vec , block_len );

            vec_len_to_go -= block_len;
        }

        matA_n_vec -= n_ele;
        b += (n_ele+1)>>1;
        matA += n_ele*matA_vec_byte;
    }
}
#undef BLOCK_LEN


void gf16mat_prod(uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b)
{
#if defined(_MUL_WITH_MULTAB_)
    gf16mat_prod_multab_impl( c, matA, n_A_vec_byte, n_A_width, b);
#else
    gf16mat_prod_neon( c, matA, n_A_vec_byte, n_A_width, b);
#endif
}

void gf16mat_rowmat_mul_ref(uint8_t *matC, const uint8_t *matA, unsigned height_A, unsigned width_A_byte, const uint8_t *matB, unsigned width_B_byte)
{
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
