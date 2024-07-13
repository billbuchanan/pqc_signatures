#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "fips202.h"
#include "impl.h"
#include "conv.h"
#include "aes256ctr.h"
#include "randvec.h"
#include "mod.h"
#include "api.h"

static const size_t d =3; //number of layers
const int64_t x_max = 4; //2 bits
const int64_t c_max = 4; //2 bits

static int64_t GG64[3*64] = {
    2, 2, 2, 1, 2, 1, 0, 0, 4, 0, 3, 2, 0, 1, 4, 0, 1, 0, 4, 0, 2, 0, 0, 4, 0, 2, 0, 1, 3, 1, 0, 0, 0, 4, 1, 0, 3, 2, 3, 0, 2, 4, 2, 3, 2, 3, 0, 1, 3, 1, 1, 3, 0, 3, 4, 2, 1, 1, 4, 4, 1, 3, 0, 2,
    397, 475, 277, 136, 224, 145, 460, 146, 541, 494, 285, 67, 363, 389, 281, 63, 175, 274, 225, 516, 266, 27, 368, 350, 366, 95, 503, 509, 554, 132, 367, 355, 271, 171, 99, 235, 237, 242, 181, 367, 36, 325, 283, 210, 294, 7, 542, 129, 154, 415, 472, 333, 335, 350, 380, 61, 246, 398, 501, 513, 554, 29, 415, 47,
    28666049, 25540221, 3555391, 21807489, 12985727, 37188019, 17496176, 7805885, 1864852, 8096492, 21219010, 64277507, 59792348, 211295, 25055402, 936334, 12988388, 39683795, 60834754, 50222767, 8217609, 38554814, 21626786, 56643630, 7977394, 28953216, 23686417, 12996552, 42706240, 17810610, 27501688, 35844836, 25984502, 32462630, 19321788, 27945324, 56321940, 14093916, 35223695, 40521611, 56580224, 60236663, 56675824, 7973116, 63300802, 46388364, 18056283, 16701327, 13570253, 53550078, 25551071, 11541856, 26312949, 44657654, 31735334, 6928092, 51890525, 9162435, 52262408, 34004380, 48078321, 20137489, 19862432, 20095321
};
static int64_t GG96[3*96] = {
    2, 2, 3, 0, 4, 2, 1, 0, 1, 2, 0, 0, 1, 2, 3, 3, 4, 2, 3, 2, 1, 0, 4, 2, 1, 3, 4, 2, 3, 2, 1, 3, 0, 1, 4, 4, 4, 2, 1, 4, 1, 4, 0, 2, 4, 0, 4, 0, 0, 3, 3, 1, 1, 0, 1, 2, 0, 2, 0, 2, 4, 1, 1, 1, 0, 1, 4, 4, 3, 3, 2, 1, 1, 4, 1, 2, 1, 1, 1, 1, 3, 1, 1, 1, 2, 0, 1, 0, 3, 4, 0, 3, 0, 2, 2, 0,
    250, 116, 299, 2, 612, 163, 220, 741, 363, 718, 576, 308, 153, 590, 357, 139, 25, 204, 580, 120, 571, 610, 414, 26, 701, 730, 14, 227, 417, 386, 617, 768, 642, 746, 316, 325, 34, 576, 781, 73, 637, 38, 168, 83, 766, 378, 19, 784, 369, 602, 626, 321, 658, 343, 304, 272, 250, 714, 731, 43, 522, 660, 591, 282, 340, 190, 455, 694, 331, 494, 514, 550, 786, 315, 675, 498, 282, 2, 403, 790, 389, 499, 750, 256, 452, 100, 233, 482, 387, 201, 146, 229, 584, 434, 505, 305,
    37953832, 140408681, 169550225, 70473324, 151606005, 209858340, 69890737, 121862994, 225067796, 111226847, 172815471, 55300058, 100004988, 150155582, 236803634, 266344916, 92208164, 126706399, 205488164, 79359924, 122839558, 83215697, 70824055, 214047525, 173516491, 86121356, 248072521, 259869043, 42766712, 170975437, 108640153, 137944780, 221246795, 47376343, 55663435, 204390974, 164742876, 148187742, 158910417, 51571945, 172005361, 24359864, 19149951, 236227821, 76581805, 135872403, 164182680, 71865806, 2498554, 116834619, 3502180, 195372951, 170763991, 52585020, 257300425, 186187430, 266139114, 194711620, 255110979, 52632210, 248881861, 163693556, 181090685, 40466257, 218040846, 148106162, 81898588, 117469909, 196923410, 39673944, 108334516, 72401625, 196659977, 94047255, 224083735, 182287100, 198931099, 216499954, 130278821, 136905208, 138687291, 245212271, 195226624, 223505525, 119061764, 268076263, 16203558, 43159063, 141999448, 216994447, 236272617, 212709968, 96594045, 68894833, 28710809, 176098627
};
static int64_t GG128[3*128] = {
    1, 1, 3, 4, 1, 1, 0, 4, 1, 4, 1, 1, 4, 2, 1, 0, 2, 1, 4, 2, 1, 0, 4, 2, 2, 2, 1, 0, 0, 3, 1, 1, 4, 0, 2, 2, 1, 4, 3, 2, 3, 1, 1, 2, 1, 4, 2, 1, 1, 3, 4, 4, 2, 2, 3, 1, 3, 4, 3, 3, 0, 1, 1, 3, 1, 1, 2, 4, 2, 2, 2, 3, 3, 1, 1, 4, 4, 0, 4, 1, 3, 0, 4, 2, 3, 1, 1, 0, 0, 0, 4, 3, 2, 1, 4, 1, 4, 4, 1, 2, 1, 4, 2, 4, 3, 4, 3, 4, 2, 1, 2, 1, 3, 3, 0, 3, 1, 4, 1, 3, 4, 3, 3, 2, 0, 3, 4, 0,
    989, 933, 659, 1046, 142, 439, 814, 798, 858, 97, 163, 339, 945, 906, 330, 909, 756, 1035, 302, 304, 881, 566, 314, 13, 1043, 310, 302, 199, 1057, 958, 423, 519, 248, 739, 482, 772, 826, 286, 30, 441, 396, 533, 330, 595, 876, 526, 311, 230, 545, 742, 115, 929, 377, 11, 515, 1073, 719, 446, 507, 167, 939, 973, 228, 208, 628, 641, 1004, 493, 846, 841, 962, 246, 572, 217, 176, 967, 150, 994, 96, 785, 35, 828, 484, 579, 131, 597, 847, 325, 332, 671, 761, 381, 1039, 244, 90, 957, 843, 226, 525, 1059, 484, 463, 711, 527, 184, 588, 385, 937, 874, 194, 95, 138, 316, 35, 1011, 26, 470, 4, 327, 421, 778, 803, 445, 400, 77, 241, 58, 587,
    491434567, 255628724, 699110100, 936209701, 120190571, 635312751, 434663218, 876634405, 299009478, 835400284, 999949652, 444524683, 510177674, 26313318, 696284846, 755240093, 671098842, 702713621, 48452012, 121660269, 434469716, 703849641, 983899662, 308653495, 449145128, 51077507, 110438815, 765318212, 912631738, 988705270, 463623347, 12535125, 501239636, 697466286, 46794805, 263509947, 79991499, 61087618, 1061095206, 1012175061, 1015240873, 740126560, 529053832, 401284272, 164241318, 592787910, 902428524, 280146359, 790175864, 1030799785, 479325744, 752923647, 249405621, 347330641, 304882506, 790466246, 643427575, 381050403, 336548129, 723012665, 189039221, 705242530, 731665143, 611948143, 104067719, 841673092, 535602351, 941820194, 124742077, 453197168, 1027082397, 559937878, 834602081, 83647918, 857678635, 596254477, 1065213431, 878546453, 493569517, 269519382, 759144535, 938867743, 692850271, 408156176, 75943054, 569885490, 1167606, 485860277, 655047104, 539771724, 385919804, 392135222, 440078135, 294703153, 799074599, 107156919, 937111729, 315344593, 86473005, 1035246855, 639190981, 62898848, 532726066, 704373414, 853578054, 784064791, 447338749, 660104013, 650104651, 787934168, 612484159, 386040162, 754845291, 1050930181, 620400889, 974640165, 90402909, 43495225, 693254602, 930109825, 421054717, 436913381, 32883751, 217098479, 939595120, 834181015, 210793194, 861425423
};

static uint64_t vc64[4]  = {503673, 952989, 557, 1120};
static uint64_t vc96[4]  = {1756408, 2988441, 1336,  2368};
static uint64_t vc128[4] = {4229853, 6822141, 2507, 4079};

static int64_t p64[3] =  {5, 557, 67108864};
static int64_t p96[3] =  {5, 823, 268435456};
static int64_t p128[3] = {5, 1097, 1073741824};

static int64_t *G[3];
static int64_t *p;
static uint64_t *vc;

void (*conv_p[3])(int64_t *, const int64_t *, const int64_t *);
void (*conv_0)(int64_t *, const int64_t *, const int64_t *);

void (*add_p[3])(int64_t *, const int64_t *, const int64_t *);

static void mkPG(const size_t n)
{
    if(n==64){
        G[0] = GG64;
        G[1] = GG64 + 64;
        G[2] = GG64 + 2*64;
        p = p64;
        vc = vc64;

        conv_p[0] = conv_64_mod_5;
        conv_p[1] = conv_64_mod_557;
        conv_p[2] = conv_64_mod_67108864;
        conv_0 = conv_64;

        add_p[0] = add_64_mod_5;
        add_p[1] = add_64_mod_557;
        add_p[2] = add_64_mod_67108864;
    }
    else if(n==96){
        G[0] = GG96;
        G[1] = GG96 + 96;
        G[2] = GG96 + 2*96;
        p = p96;
        vc = vc96;

        conv_p[0] = conv_96_mod_5;
        conv_p[1] = conv_96_mod_823;
        conv_p[2] = conv_96_mod_268435456;
        conv_0 = conv_96;

        add_p[0] = add_96_mod_5;
        add_p[1] = add_96_mod_823;
        add_p[2] = add_96_mod_268435456;
    }
    else if(n==128){
        G[0] = GG128;
        G[1] = GG128 + 128;
        G[2] = GG128 + 2*128;
        p = p128;
        vc = vc128;

        conv_p[0] = conv_128_mod_5;
        conv_p[1] = conv_128_mod_1097;
        conv_p[2] = conv_128_mod_1073741824;
        conv_0 = conv_128;

        add_p[0] = add_128_mod_5;
        add_p[1] = add_128_mod_1097;
        add_p[2] = add_128_mod_1073741824;
    }
}

size_t pack_u(unsigned char *r, const int64_t u[N_MAX], const size_t n)
{
    size_t i;

    for (i = 0; i < n >> 3; i++)
    {
        r[0]=u[(i<<3)];
        r[1]=u[(i<<3)]>>8;
        r[2]=u[(i<<3)]>>16;
        r[3]=(u[(i<<3)]>>24)|(u[(i<<3)+1]<<2);
        r[4]=u[(i<<3)+1]>>6;
        r[5]=u[(i<<3)+1]>>14;
        r[6]=(u[(i<<3)+1]>>22)|(u[(i<<3)+2]<<4);
        r[7]=u[(i<<3)+2]>>4;
        r[8]=u[(i<<3)+2]>>12;
        r[9]=(u[(i<<3)+2]>>20)|(u[(i<<3)+3]<<6);
        r[10]=u[(i<<3)+3]>>2;
        r[11]=u[(i<<3)+3]>>10;
        r[12]=u[(i<<3)+3]>>18;
        r[13]=u[(i<<3)+4];
        r[14]=u[(i<<3)+4]>>8;
        r[15]=u[(i<<3)+4]>>16;
        r[16]=(u[(i<<3)+4]>>24)|(u[(i<<3)+5]<<2);
        r[17]=u[(i<<3)+5]>>6;
        r[18]=u[(i<<3)+5]>>14;
        r[19]=(u[(i<<3)+5]>>22)|(u[(i<<3)+6]<<4);
        r[20]=u[(i<<3)+6]>>4;
        r[21]=u[(i<<3)+6]>>12;
        r[22]=(u[(i<<3)+6]>>20)|(u[(i<<3)+7]<<6);
        r[23]=u[(i<<3)+7]>>2;
        r[24]=u[(i<<3)+7]>>10;
        r[25]=u[(i<<3)+7]>>18;

        r += 26;
    }

    return 26 * (n >> 3);
}

void pack_pk(unsigned char *pk_out, const pubkey_t *pk_in, const size_t n)
{
    pk_out += pack_u(pk_out, pk_in->h1, n);
    pack_u(pk_out, pk_in->h2, n);
}

static void hashVec(int64_t* c1, int64_t* c2, const uint8_t *m, const size_t mlen, const int64_t *u, const uint8_t *pkh, const size_t n)
{
    const size_t ulen = 208;
    uint8_t *hash_in = (uint8_t *)malloc(mlen + ulen + n / 2);
    uint8_t hashc[64];
    size_t i, hash_len;

    memcpy(hash_in, m, mlen);
    memcpy(hash_in + mlen, pkh, n / 2);
    hash_len = mlen + n / 2;

    if (u != NULL)
    {
        pack_u(hash_in + mlen + n / 2, u, n);
        hash_len += ulen;
    }

    if(n==64){
        sha3_256(hashc, hash_in, hash_len);
        for (i=0; i< 16; i++){
            c1[4*i+0] = (hashc[i]>>0)&0x03;
            c1[4*i+1] = (hashc[i]>>2)&0x03;
            c1[4*i+2] = (hashc[i]>>4)&0x03;
            c1[4*i+3] = (hashc[i]>>6)&0x03;
        }
        for (i=0; i< 16; i++){
            c2[4*i+0] = (hashc[16+i]>>0)&0x03;
            c2[4*i+1] = (hashc[16+i]>>2)&0x03;
            c2[4*i+2] = (hashc[16+i]>>4)&0x03;
            c2[4*i+3] = (hashc[16+i]>>6)&0x03;
        }
    }
    else if(n==96){
        sha3_384(hashc, hash_in, hash_len);
        for (i=0; i< 24; i++){
            c1[4*i+0] = (hashc[i]>>0)&0x03;
            c1[4*i+1] = (hashc[i]>>2)&0x03;
            c1[4*i+2] = (hashc[i]>>4)&0x03;
            c1[4*i+3] = (hashc[i]>>6)&0x03;
        }
        for (i=0; i< 24; i++){
            c2[4*i+0] = (hashc[24+i]>>0)&0x03;
            c2[4*i+1] = (hashc[24+i]>>2)&0x03;
            c2[4*i+2] = (hashc[24+i]>>4)&0x03;
            c2[4*i+3] = (hashc[24+i]>>6)&0x03;
        }
    }
    else if(n==128){
        sha3_512(hashc, hash_in, hash_len);
        for (i=0; i< 32; i++){
            c1[4*i+0] = (hashc[i]>>0)&0x03;
            c1[4*i+1] = (hashc[i]>>2)&0x03;
            c1[4*i+2] = (hashc[i]>>4)&0x03;
            c1[4*i+3] = (hashc[i]>>6)&0x03;
        }
        for (i=0; i< 32; i++){
            c2[4*i+0] = (hashc[32+i]>>0)&0x03;
            c2[4*i+1] = (hashc[32+i]>>2)&0x03;
            c2[4*i+2] = (hashc[32+i]>>4)&0x03;
            c2[4*i+3] = (hashc[32+i]>>6)&0x03;
        }
    }
    free(hash_in);
}

static inline int64_t sumList(const int64_t* a, const size_t n){
    int64_t sum = 0;
    size_t i;
    for(i=0; i < n; i++){
        sum = sum + a[i];
    }
    return sum;
}

static inline void vecScalar(int64_t* a, const int64_t c, const size_t n){
    size_t i;
    for (i=0; i < n; i++){
        a[i] = a[i]*c;
    }
}

static inline void vecDiv(int64_t* a, const int64_t c, const size_t n){
    size_t i;
    for (i=0; i < n; i++){
        a[i] = a[i]/c;
    }
}

static inline uint64_t vecNorm2(const int64_t* a,  const size_t n){
    size_t i; uint64_t sum=0;
    for (i=0; i < n; i++){
        sum = sum + a[i]*a[i];
    }

    return sum;
}

static inline uint64_t vecVar(const int64_t* a, const size_t n){
    size_t i; int64_t sum, v;
    sum = sumList(a, n);
    v = sum/n;
    sum =0;
    for (i=0; i < n; i++){
        sum = sum + (a[i]-v)*(a[i]-v);
    }

    return sum;
}

static inline void add(const int64_t* a, const int64_t* b, int64_t* c, const size_t n){
    size_t i;
    for (i=0; i < n; i++){
        c[i] = a[i] + b[i];
    }
}



static inline int ckSize(const int64_t* a, const int64_t min, const int64_t max, const size_t n){
    size_t i;
    for(i=0; i < n; i++){
        if(a[i]<min || a[i]>max)
            return 0;
    }
    return 1;
}

static int64_t randomize(int64_t* h, const size_t n, const size_t l, const int a, aes256ctr_ctx *ctx){
    size_t i, j;
    int64_t pos[N_MAX] = {0};
    size_t w[N_MAX];
    int64_t dist[(N_MAX >> 1) + 2], noise[N_MAX];
    size_t num;
    int64_t sumR = 0;

    num = (p[l+1] - sumList(h, n)*(c_max-1))/((c_max)*p[l]);
    if (a) num*=2;

    randVec_loc(w, n, ctx);
    randVec_dist(dist, n >> 1, num, ctx);
    for(j=0; j < (n >> 1) + 2; j++){
        for (i=0; i<n; i++){
            pos[i] += (-(1 ^ (ct_neq(i, w[j]) >> 63))) & dist[j];
        }
    }
    randVec_noise(noise, n, a, ctx);
    for (i=0; i<n; i++){
        if (!a)
        {
            sumR += (-(1 ^ (ct_neq(pos[i], 0) >> 63))) & noise[i];
        }
        pos[i] += (-(1 ^ (ct_neq(pos[i], 0) >> 63))) & noise[i];
    }

    for (j=0; j<n; j++){
        h[j] = h[j]+pos[j]*p[l];
    }

    return sumR;
}

static int64_t eMLE(const int64_t* x, int64_t* h, int64_t F[][N_MAX], const size_t n, const int64_t* o, const int a, aes256ctr_ctx *ctx){
    size_t l;
    int64_t sumR = 0;
    memset(h, 0, n*sizeof(int64_t));

    int64_t c[N_MAX], c1[N_MAX];

    for(l=0; l < d; l++){
        if(l==0){
            add_p[l](c1, x, o);
            conv_p[l](c, G[l], c1);
        }
        else
            conv_p[l](c, G[l], x);

        add_p[l](h, h, c);
        if(l<d-1){
            if(l==d-2) sumR = randomize(h, n, l, a, ctx);
            memcpy(F[l], h, n*sizeof(int64_t));
        }
    }
    return sumR;
}

static void gen_pkh(uint8_t *pkh, const pubkey_t* pk, const size_t n)
{
    unsigned char pk_out[CRYPTO_PUBLICKEYBYTES];

    pack_pk(pk_out, pk, n);
    if(n==64){
        sha3_256(pkh, pk_out, CRYPTO_PUBLICKEYBYTES);
    }
    else if(n==96){
        sha3_384(pkh, pk_out, CRYPTO_PUBLICKEYBYTES);
    }
    else if(n==128){
        sha3_512(pkh, pk_out, CRYPTO_PUBLICKEYBYTES);
    }
}

void keygen(pubkey_t* pk, privkey_t* sk, const size_t n, const uint8_t seed[32]){
    aes256ctr_ctx ctx;
    uint8_t nonce[12]={0};

    mkPG(n);

    aes256ctr_init(&ctx, seed, nonce);

    int64_t sumX;
    do {
        randVec_xmax(sk->x1, n, &ctx);
        randVec_xmax(sk->x2, n, &ctx);
        sumX = sumList(sk->x1, n) + sumList(sk->x2, n);
        sumX ^= (-(ct_lt(sumX, 0) >> 63)) & (sumX ^ (-sumX));
    } while (1 ^ (ct_lt(sumX, n/2) >> 63));

    int64_t sumR;
    do {
        sumR = eMLE(sk->x1, pk->h1, sk->F1, n, G[1], 0, &ctx) + eMLE(sk->x2, pk->h2, sk->F2, n, G[1], 0, &ctx);
        sumR ^= (-(ct_lt(sumR, 0) >> 63)) & (sumR ^ (-sumR));
    } while (1 ^ (ct_lt(sumR, n*n) >> 63));

    gen_pkh(sk->pkh, pk, n);
}

static int checkS(const signature_t* sig, const size_t n){
    if(!ckSize(sig->s, 0, n*c_max*x_max/2-1, n))
        return 0;

    uint64_t var = vecVar(sig->s, n);
    return (var >= vc[0]) && (var <= vc[1]);
}

static int checkL0(const signature_t* sig, int64_t* c1, int64_t* c2, const int64_t* cp, const int64_t* F0, const size_t n){//c1 also contains returned value
    add(c1, c2, c1, n);
    conv_p[0](c2, G[1], c1);
    add(sig->s, c2, c1, n);
    add(c1, cp, c2, n);

    conv_p[0](c1, G[0], c2);
    vecScalar(c1, -1, n);
    add(F0, c1, c2, n);
    vecDiv(c2, p[0], n);

    uint64_t var = vecVar(c2, n);
    return (var <= vc[3]) && (var >= vc[2]);
}

static int check(const privkey_t* sk, const signature_t* sig, int64_t* c1, int64_t* c2, const int64_t* cp, int64_t F[][N_MAX], const size_t n){
    int64_t c[N_MAX];
    int l;
    if(!checkS(sig, n)) return 0;

    for(l=d-2; l >= 0; l--){
        conv_0(c, sk->F1[l], c1);
        add(F[l], c, F[l], n);
        conv_0(c, sk->F2[l], c2);
        add(F[l], c, F[l], n);
        if(!ckSize(F[l], 0, p[l+1]-1, n))
            return 0;

        if(l==0){
            return checkL0(sig, c1, c2, cp, F[0], n);
        }
    }
}

void sign(signature_t* sig, const privkey_t* sk, const uint8_t* m, const size_t mlen, const size_t n, const uint8_t seed[32]){
    int64_t c[N_MAX], c1[N_MAX], c2[N_MAX], cp[N_MAX];
    int64_t sumXn  = 0, sumXp  = 0; size_t i;

    int64_t F[2][N_MAX];

    aes256ctr_ctx ctx;
    uint8_t nonce[12]={0};

    mkPG(n);

    aes256ctr_init(&ctx, seed, nonce);

    for(i=0; i<n; i++){
        sumXn-=((-(ct_lt(sk->x1[i], 0)>>63))&sk->x1[i])+((-(ct_lt(sk->x2[i], 0)>>63))&sk->x2[i]);
        sumXp+=((-(ct_lt(0, sk->x1[i])>>63))&sk->x1[i])+((-(ct_lt(0, sk->x2[i])>>63))&sk->x2[i]);
    }

    hashVec(c1, c2, m, mlen, NULL, sk->pkh, n);
    add(c1, c2, cp, n);

    int64_t y[N_MAX];
    do {
        randVec_y(y, n, sumXn, sumXp, &ctx);
        eMLE(y, sig->u, F, n, cp, 1, &ctx);

        hashVec(c1, c2, m, mlen, sig->u, sk->pkh, n);

        conv_0(sig->s, c1, sk->x1);
        conv_0(c, sk->x2, c2);
        add(sig->s, c, sig->s, n);
        add(sig->s, y, sig->s, n);
    } while (!check(sk, sig, c1, c2, cp, F, n));
}

int verify(const pubkey_t* pk, const uint8_t* m, const size_t mlen, const signature_t* sig, const size_t n){
    int64_t c[N_MAX], c1[N_MAX], c2[N_MAX];
    int l;

    mkPG(n);

    uint8_t pkh[64];
    gen_pkh(pkh, pk, n);

    int64_t t[N_MAX] = {0}, cp[N_MAX];
    hashVec(c1, c2, m, mlen, NULL, pkh, n);
    add(c1, c2, cp, n);

    hashVec(c1, c2, m, mlen, sig->u, pkh, n);
    int v = checkS(sig, n);

    conv_p[d-1](t, pk->h1, c1);
    conv_p[d-1](c, pk->h2, c2);
    add_p[d-1](t, t, c);
    add_p[d-1](t, t, sig->u);

    for(l=d-1; l >=0; l--){
        if(l==0){
            v = v && checkL0(sig, c1, c2, cp, t, n);
            add_p[l](t, t, c1);
        }
        else{
            conv_p[l](c, G[l], sig->s);
            vecScalar(c, -1, n);
            add_p[l](t, t, c);
        }
    }
    v = v&&ckSize(t, 0, 0, n);
    return v;
}
