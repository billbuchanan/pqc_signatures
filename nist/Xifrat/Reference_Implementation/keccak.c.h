/* DannyNiu/NJF, 2018-02-06. Public Domain. */

#if defined(Keccak_InstName) && defined(keccak_word_t)

#define w (sizeof(keccak_word_t) * 8)
#define l (w==64?6 : w==32?5 : w==16?4 : w==8?3 : w==4?2 : w==2?1 : 0)

typedef keccak_word_t keccak_state_t[5][5];

// #define A(x,y,z) ((A[y][x]>>z)&1) // helps you remember.

static inline keccak_word_t rot(keccak_word_t x, int s) // rotate-left.
{
    register unsigned u = (unsigned)s & (w-1);
    return u ? ( x << u ) | ( x >> (w-u) ) : x;
}

static inline int mod5(int x) // x shall always be data-independent.
{
    // ad-hoc, but efficient here.
    while( x >= 5 ) x -= 10;
    while( x < 0 ) x += 5;
    return x;
}

#include "endian.h"

// 2020-07-09:
// This isn't the correct way to use ``_Generic'',
// but exception is allowed here for the ``uint8_t'' case.

#define letoh(x)                                \
    _Generic(x,                                 \
             uint8_t:(x),                       \
             uint16_t:le16toh(x),               \
             uint32_t:le32toh(x),               \
             uint64_t:le64toh(x)                \
        )
#define htole(x)                                \
    _Generic(x,                                 \
             uint8_t:(x),                       \
             uint16_t:htole16(x),               \
             uint32_t:htole32(x),               \
             uint64_t:htole64(x)                \
        )

static inline void theta(keccak_state_t A_out, keccak_state_t A)
{
    keccak_word_t C[5];
    register int x, y;

    for(x=0; x<5; x++)
        C[x] =
            A[0][x] ^
            A[1][x] ^
            A[2][x] ^
            A[3][x] ^
            A[4][x] ;

    for(x=0; x<5; x++)
    {
        keccak_word_t D = C[ mod5(x-1) ] ^ rot( C[ mod5(x+1) ] , 1 );
        for(y=0; y<5; y++) A_out[y][x] = A[y][x] ^ D;
    }
}

static inline void rho(keccak_state_t A_out, keccak_state_t A)
{
    int x=1, y=0;
    int t;

    A_out[0][0] = A[0][0];

    for(t=0; t<24; t++)
    {
        int x2 = y, y2 = mod5( 2*x+3*y );
        A_out[y][x] = rot( A[y][x] , ((t+1)*(t+2))>>1 );

        x = x2;
        y = y2;
    }
}

static inline void pi(keccak_state_t A_out, keccak_state_t A)
{
    int x, y;
    for(y=0; y<5; y++)
        for(x=0; x<5; x++)
            A_out[y][x] = A[x][ mod5(x+3*y) ];
}

static inline void chi(keccak_state_t A_out, keccak_state_t A)
{
    int x, y;
    for(y=0; y<5; y++)
        for(x=0; x<5; x++)
            A_out[y][x] = A[y][x] ^ ( ~A[y][ mod5(x+1) ] & A[y][ mod5(x+2) ] );
}

static inline int iota(keccak_state_t A_out, keccak_state_t A, int lfsr)
{
    int x, y;
    int j;
    keccak_word_t RC = 0;
    
    for(y=0; y<5; y++)
        for(x=0; x<5; x++)
            A_out[y][x] = A[y][x];

    for(j=0; j<=6; j++)
    {
        if( j <= l )
            RC ^= ((keccak_word_t)lfsr&1) << ((1<<j)-1);

        lfsr <<= 1;
        lfsr ^= ((lfsr>>8)&1)*0x71;
        lfsr &= 0xff;
    }

    A_out[0][0] ^= RC;
    return lfsr;
}

// Intentionally not restrict-qualified. 
void glue(Keccak_InstName,_Permute)(void const *in, void *out)
{
    keccak_state_t A;
    keccak_word_t const *cptr;
    keccak_word_t *ptr;
    int lfsr = 1;

    int x, y;
    int ir;

    cptr = in;
    for(y=0; y<5; y++)
        for(x=0; x<5; x++)
            A[y][x] = letoh(cptr[y*5+x]);

    // This version of MySuiteA implements Keccak-f,
    // and the generalized Keccak-p is omitted.
    for(ir = 0; ir < 12+2*l; ir++) {
        theta(A, A);
        rho(out, A);
        pi(A, out);
        chi(out, A);
        lfsr = iota(A, out, lfsr);
    }

    ptr = out;
    for(y=0; y<5; y++)
        for(x=0; x<5; x++)
            ptr[y*5+x] = htole(A[y][x]);
}

IntPtr glue(i,Keccak_InstName)(int q){ return glue(c,Keccak_InstName)(q); }

#undef Keccak_InstName
#undef keccak_word_t
#undef w
#undef l
#endif /* Keccak_InstName */
