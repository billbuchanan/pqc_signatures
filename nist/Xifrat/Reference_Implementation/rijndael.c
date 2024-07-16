/* DannyNiu/NJF, 2018-01-31. Public Domain. */

#include "rijndael.h"
#include "endian.h"

static const alignas(256) uint8_t sbox_table[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16,
};

static inline uint8_t sbox(uint8_t x, uint8_t const sbox_table[256])
{
    int i;
    uint8_t ret = 0;
    uint16_t mask = 0;

    for(i=0; i<256; i++)
    {
        mask = i ^ x;
        mask = (mask - 1) >> 8;
        ret |= sbox_table[i] & mask;
    }

    return ret;
}

static inline uint8_t invsbox(uint8_t x, uint8_t const sbox_table[256])
{
    int i;
    uint8_t ret = 0;
    uint16_t mask = 0;

    for(i=0; i<256; i++)
    {
        mask = sbox_table[i] ^ x;
        mask = (mask - 1) >> 8;
        ret |= i & mask;
    }

    return ret;
}

static inline uint8_t xtime(uint16_t x)
{
    x = ((x << 1) & 0x00ff) - ((x << 1) & 0x0100);
    return (uint8_t)(x ^ ((x >> 8) & 0x1b));
}

// #ifndef Define_AES_Cipher // 2022-06-27: AES Dual-Implementation.

static inline uint8_t gmul(uint8_t a, uint8_t b)
{
    register uint8_t x = 0;
    register int i;

    // 2021-09-02.
    // Per https://crypto.stackexchange.com/a/82102/36960,
    // one of the operands of ``gmul'' is actually from
    // a set of public constants, where there's no need to
    // mask for time side-channels.

    for(i=0; i<8; a=xtime(a),i++)
        if( 1 & (b >> i) ) x ^= a;

    return x;
}

// column-major indexing.
#define s(r,c) (state[r+c*4])

static void SubBytes(uint8_t state[16])
{
    int i;
    for(i=0; i<16; i++) state[i] = sbox(state[i], sbox_table);
}

static void ShiftRows(uint8_t state[16])
{
    uint8_t x;

    x = s(1,0);
    s(1,0) = s(1,1);
    s(1,1) = s(1,2);
    s(1,2) = s(1,3);
    s(1,3) = x;

    x = s(2,0);
    s(2,0) = s(2,2);
    s(2,2) = x;
    x = s(2,1);
    s(2,1) = s(2,3);
    s(2,3) = x;

    x = s(3,3);
    s(3,3) = s(3,2);
    s(3,2) = s(3,1);
    s(3,1) = s(3,0);
    s(3,0) = x;
}

static void MixColumns(uint8_t state[16])
{
    static const uint8_t a[4] = {0x03, 0x01, 0x01, 0x02};
    auto uint8_t s2[16] = {0};
    register int c, r, i;

    for(c=0; c<4; c++)
	for(r=0; r<4; r++)
            for(i=0; i<4; i++)
		s2[r+c*4] ^= gmul(s(i,c), a[(i+7-r)%4]);

    for(i=0; i<16; i++)
        state[i] = s2[i];
}

static void AddRoundKey(uint8_t state[16], uint8_t const w[16])
{
    int i;
    for(i=0; i<16; i++)
        state[i] ^= w[i];
}

static void InvShiftRows(uint8_t state[16])
{
    uint8_t x;

    x = s(1,3);
    s(1,3) = s(1,2);
    s(1,2) = s(1,1);
    s(1,1) = s(1,0);
    s(1,0) = x;

    x = s(2,3);
    s(2,3) = s(2,1);
    s(2,1) = x;
    x = s(2,2);
    s(2,2) = s(2,0);
    s(2,0) = x;

    x = s(3,0);
    s(3,0) = s(3,1);
    s(3,1) = s(3,2);
    s(3,2) = s(3,3);
    s(3,3) = x;
}

static void InvSubBytes(uint8_t state[16])
{
    int i;
    for(i=0; i<16; i++) state[i] = invsbox(state[i], sbox_table);
}

static void InvMixColumns(uint8_t state[16])
{
    static const uint8_t a[4] = {0x0b, 0x0d, 0x09, 0x0e};
    auto uint8_t s2[16] = {0};
    register int c, r, i;

    for(c=0; c<4; c++)
	for(r=0; r<4; r++)
            for(i=0; i<4; i++)
		s2[r+c*4] ^= gmul(s(i,c), a[(i+7-r)%4]);

    for(i=0; i<16; i++)
        state[i] = s2[i];
}

// In the following definitions, `in' and `out' are intentionally
// not restrict-qualified.

#define Define_AES_Cipher(name,Nr)              \
    void name(void const *in, void *out,        \
              void const *restrict w)           \
    {                                           \
        Rijndael_Nb4_Cipher(in, out, w, Nr);    \
    }
static void Rijndael_Nb4_Cipher(
    uint8_t const in[16], uint8_t out[16],
    uint8_t const *restrict w, int Nr)
{
    register int i;

    for(i=0; i<16; i++) out[i] = in[i];

    AddRoundKey(out, w);

    for(i=1; i<Nr; i++) {
        SubBytes(out);
        ShiftRows(out);
        MixColumns(out);
        AddRoundKey(out, w + i*16);
    }

    SubBytes(out);
    ShiftRows(out);
    AddRoundKey(out, w + Nr*16);
}

#define Define_AES_InvCipher(name,Nr)           \
    void name(void const *in, void *out,        \
              void const *restrict w)           \
    {                                           \
        Rijndael_Nb4_InvCipher(in, out, w, Nr); \
    }
static void Rijndael_Nb4_InvCipher(
    uint8_t const in[16], uint8_t out[16],
    uint8_t const *restrict w, int Nr)
{
    register int i;

    for(i=0; i<16; i++) out[i] = in[i];

    AddRoundKey(out, w + Nr*16);

    for(i=Nr; --i>0; ) {
        InvShiftRows(out);
        InvSubBytes(out);
        AddRoundKey(out, w + i*16);
        InvMixColumns(out);
    }

    InvShiftRows(out);
    InvSubBytes(out);
    AddRoundKey(out, w);
}

// #endif /* Define_AES_Cipher */ // AES Dual-Implementation

#define Define_AES_KeyExpansion(name,Nk,Nr)             \
    void name(void const *restrict key,                 \
              void *restrict w)                         \
    {                                                   \
        Rijndael_Nb4_KeyExpansion(key, w, Nk, Nr);      \
    }
static void Rijndael_Nb4_KeyExpansion(
    uint8_t const *restrict key_in, uint8_t *restrict w_out,
    int Nk, int Nr)
{
    uint8_t Rcon = 0x01;
    uint32_t temp;
    uint32_t const *key = (const void *)key_in;
    uint32_t *w = (void *)w_out;
    int i;

    for(i=0; i<Nk; i++)
        w[i] = key[i];

    for(i=Nk; i < 4*(Nr+1); i++) {
        temp = le32toh( w[i-1] );

        if( i % Nk == 0 )
        {
            temp = (
                (uint32_t)sbox(temp >> 24, sbox_table) << 16 |
                (uint32_t)sbox(temp >> 16, sbox_table) <<  8 |
                (uint32_t)sbox(temp >>  8, sbox_table) <<  0 |
                (uint32_t)sbox(temp >>  0, sbox_table) << 24 ) ^ Rcon;
            Rcon = xtime(Rcon);
        }
        else if( Nk > 6 && i % Nk == 4 )
        {
            temp =
                (uint32_t)sbox(temp >> 24, sbox_table) << 24 |
                (uint32_t)sbox(temp >> 16, sbox_table) << 16 |
                (uint32_t)sbox(temp >>  8, sbox_table) <<  8 |
                (uint32_t)sbox(temp >>  0, sbox_table) <<  0 ;
        }

        w[i] = w[i - Nk] ^ htole32(temp);
    }
}

Define_AES_Cipher(AES128_Cipher,10);
Define_AES_Cipher(AES192_Cipher,12);
Define_AES_Cipher(AES256_Cipher,14);

Define_AES_InvCipher(AES128_InvCipher,10);
Define_AES_InvCipher(AES192_InvCipher,12);
Define_AES_InvCipher(AES256_InvCipher,14);

Define_AES_KeyExpansion(AES128_KeyExpansion,4,10);
Define_AES_KeyExpansion(AES192_KeyExpansion,6,12);
Define_AES_KeyExpansion(AES256_KeyExpansion,8,14);

IntPtr iAES128(int q) { return xAES128(q); }
IntPtr iAES192(int q) { return xAES192(q); }
IntPtr iAES256(int q) { return xAES256(q); }
