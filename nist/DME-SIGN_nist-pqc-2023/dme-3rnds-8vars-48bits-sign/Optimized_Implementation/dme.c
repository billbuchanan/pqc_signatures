#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "rng.h"
#include "dme.h"

fq_elem fq_add(fq_elem a, fq_elem b)
{
    return a ^ b;
}

fq_elem fq_mul(fq_elem a, fq_elem b)
{
    fq_elem c;
    asm
    (
        "movq $0x20007, %0\n"
        "movq %0, %%xmm1\n"
        "pclmullqlqdq %2, %1\n"
        "movq %1, %0\n"
        "psrldq $0x06, %1\n"
        "pclmullqlqdq %%xmm1, %1\n"
        "movq %1, %%r8\n"
        "xorq %%r8, %0\n"
        "psrldq $0x06, %1\n"
        "pclmullqlqdq %%xmm1, %1\n"
        "movq %1, %%r8\n"
        "xorq %%r8, %0\n"
        : "=r" (c), "+x" (a)
        : "x"  (b)
        : "%xmm1", "%r8"
    );
    return c & fq_qm1;
}

fq_elem fq_sqr(fq_elem a)
{
    return fq_mul(a, a);
}

fq_elem fq_mul_t(fq_elem a)
{
    return ((a << 1) & fq_qm1) ^ ((-(a >> 47)) & fq_t48);
}

fq_elem fq_pow(fq_elem a, fq_expn n)
{
    fq_elem c;
    unsigned int i;
    c = fq_1;
    for (i=0; i<48; i++)
    {
        c = fq_sqr(c);
        if ((n >> 47) & 1)
            c = fq_mul(a, c);
        n <<= 1;
    }
    return c;
}

fq_elem fq_inv(fq_elem a)
{
    return fq_pow(a, fq_qm2);
}

fq_elem fq_pow_2_k(fq_elem a, unsigned int k)
{
    while (k--)
        a = fq_sqr(a);
    return a;
}

fq_elem fq_rnd(void)
{
    fq_elem a;
    randombytes((unsigned char*)&a, 8);
    return a & fq_qm1;
}

void fq_print(fq_elem a)
{
    unsigned int i, first;
    printf("[");
    first = 1;
    for (i=0; i<48; i++)
    {
        if (a >> 47)
        {
            if (!first)
                printf("+");
            if (i < 46)
                printf("t^%d", 47-i);
            else if (i == 46)
                printf("t");
            else
                printf("1");
            first = 0;
        }
        a <<= 1;
    }
    if (first)
        printf("0");
    printf("]");
}

const unsigned char *fq_parse(fq_elem *x, const unsigned char *s)
{
    unsigned int i;
    if (!s)
        return NULL;
    *x = 0;
    for (i=0; i<6; i++)
        *x += (fq_elem)(*s++) << (8*i);
    return s;
}

unsigned char *fq_serialize(unsigned char *s, fq_elem x)
{
    unsigned int i;
    for (i=0; i<6; i++)
        *s++ = x >> (8*i);
    return s;
}

const fq2_elem fq2_0    = { fq_0, fq_0 };
const fq2_elem fq2_1    = { fq_1, fq_0 };
const fq2_elem fq2_u    = { fq_0, fq_1 };
const fq2_elem fq2_u2   = { fq_1, fq_t };  /* Fq2 = Fq[u]/<u^2+t*u+1> */
const fq2_expn fq2_q2m1 = { fq_qm1, fq_qm1 };
const fq2_expn fq2_q2m2 = { fq_qm2, fq_qm1 };
const fq2_expn fq2_inv_delta = {
    UINT64_C(0x410420821041),
    UINT64_C(0x082104108208)
};

void fq2_set(fq_elem *a, const fq_elem *b)
{
    a[0] = b[0];
    a[1] = b[1];
}

void fq2_add(fq_elem *a, const fq_elem *b, const fq_elem *c)
{
    a[0] = fq_add(b[0], c[0]);
    a[1] = fq_add(b[1], c[1]);
}

void fq2_mul(fq_elem *a, const fq_elem *b, const fq_elem *c)
{
    fq_elem d[3];
    d[0] = fq_mul(b[0], c[0]);
    d[1] = fq_add(fq_mul(b[0], c[1]), fq_mul(b[1], c[0]));
    d[2] = fq_mul(b[1], c[1]);
    a[0] = fq_add(d[0], d[2]);
    a[1] = fq_add(d[1], fq_mul_t(d[2]));
}

void fq2_sqr(fq_elem *a, const fq_elem *b)
{
    fq_elem d;
    d = fq_sqr(b[1]);
    a[0] = fq_add(fq_sqr(b[0]), d);
    a[1] = fq_mul_t(d);
}

void fq2_pow(fq_elem *a, const fq_elem *b, const fq_expn *c)
{
    fq2_elem d;
    fq_expn n;
    unsigned int i;
    fq2_set(d, fq2_1);
    n = c[1];
    for (i=0; i<48; i++)
    {
        fq2_mul(d, d, d);
        if ((n >> 47) & 1)
            fq2_mul(d, d, b);
        n <<= 1;
    }
    n = c[0];
    for (i=0; i<48; i++)
    {
        fq2_mul(d, d, d);
        if ((n >> 47) & 1)
            fq2_mul(d, d, b);
        n <<= 1;
    }
    fq2_set(a, d);
}

void fq2_inv(fq_elem *a, const fq_elem *b)
{
    fq_elem det1, det2, det3, det, inv;
    det1 = fq_add(b[0], fq_mul_t(b[1]));
    det2 = fq_mul(det1, b[0]);
    det3 = fq_mul(b[1], b[1]);
    det  = fq_add(det2, det3);
    inv  = fq_inv(det);
    a[0] = fq_mul(det1, inv);
    a[1] = fq_mul(b[1], inv);
}

void fq2_pow_2_k(fq_elem *a, const fq_elem *b, unsigned int k)
{
    fq2_elem d;
    fq2_set(d, b);
    while (k--)
        fq2_mul(d, d, d);
    fq2_set(a, d);
}

void fq2_rnd(fq_elem *a)
{
    a[0] = fq_rnd();
    a[1] = fq_rnd();
}

void fq2_print(const fq_elem *a)
{
    printf("[");
    fq_print(a[1]);
    printf("*u+");
    fq_print(a[0]);
    printf("]");
}

const unsigned char *fq2_parse(fq_elem *x, const unsigned char *s)
{
    s = fq_parse(&x[0], s);
    s = fq_parse(&x[1], s);
    return s;
}

unsigned char *fq2_serialize(unsigned char *s, const fq_elem *x)
{
    s = fq_serialize(s, x[0]);
    s = fq_serialize(s, x[1]);
    return s;
}

int  fq_matrix_2x2_inv(fq_elem *inv, const fq_elem *mat)
{
    fq_elem dt, di, tmp;
    dt = fq_add(fq_mul(mat[0], mat[3]), fq_mul(mat[1], mat[2]));
    if (!dt) return 1;
    di = fq_inv(dt);
    tmp    = fq_mul(di, mat[3]);
    inv[1] = fq_mul(di, mat[1]);
    inv[2] = fq_mul(di, mat[2]);
    inv[3] = fq_mul(di, mat[0]);
    inv[0] = tmp;
    return 0;
}

void fq_matrix_2x2_map(fq_elem *x, const fq_elem *m, const fq_elem *y)
{
    fq_elem tmp1, tmp2;
    tmp1 = fq_add(fq_mul(m[0], y[0]), fq_mul(m[1], y[1]));
    tmp2 = fq_add(fq_mul(m[2], y[0]), fq_mul(m[3], y[1]));
    x[0] = tmp1;
    x[1] = tmp2;
}

void fq_matrix_2x2_rnd(fq_elem *mat, fq_elem *inv)
{
    do
    {
        mat[0] = fq_rnd();
        mat[1] = fq_rnd();
        mat[2] = fq_rnd();
        mat[3] = fq_rnd();
    }
    while (fq_matrix_2x2_inv(inv, mat));
}

const unsigned char *fq_matrix_2x2_parse(fq_elem *m, const unsigned char *s)
{
    s = fq_parse(&m[0], s);
    s = fq_parse(&m[1], s);
    s = fq_parse(&m[2], s);
    s = fq_parse(&m[3], s);
    return s;
}

unsigned char *fq_matrix_2x2_serialize(unsigned char *s, const fq_elem *m)
{
    s = fq_serialize(s, m[0]);
    s = fq_serialize(s, m[1]);
    s = fq_serialize(s, m[2]);
    s = fq_serialize(s, m[3]);
    return s;
}

void generate_skey(struct skey_t *sk)
{
    unsigned int i;
    memset(sk, 0, sizeof(struct skey_t));
    for (i=0; i<4; i++)
    {
        fq_matrix_2x2_rnd(&sk->L1[i][0][0], &sk->L1_inv[i][0][0]);
        fq_matrix_2x2_rnd(&sk->L2[i][0][0], &sk->L2_inv[i][0][0]);
        fq_matrix_2x2_rnd(&sk->L3[i][0][0], &sk->L3_inv[i][0][0]);
        fq_matrix_2x2_rnd(&sk->L4[i][0][0], &sk->L4_inv[i][0][0]);
    }
    for (i=0; i<4; i++)
    {
        fq2_rnd(sk->A2[i]);
        fq2_rnd(sk->A3[i]);
        fq2_rnd(sk->A4[i]);
    }
    randombytes(sk->a, 6);
    randombytes(sk->b, 6);
    randombytes(sk->c, 8);
    sk->a[0] %= 96;
    sk->a[1] %= 96;
    sk->a[2] %= 96;
    sk->a[3] %= 96;
    sk->a[4] %= 96;
    sk->a[5] %= 96;
    sk->b[0] %= 96;
    sk->b[1] %= 96;
    sk->b[2] %= 96;
    sk->b[3] %= 96;
    sk->b[4] %= 96;
    sk->b[5] %= 96;
    sk->c[0] %= 96;
    sk->c[2] %= 96;
    sk->c[3] %= 96;
    sk->c[5] %= 96;
    sk->c[6] %= 96;
    sk->c[1] = (sk->a[0] + sk->b[0] + sk->c[0] - sk->a[1] - sk->b[2] + 192) % 96;
    sk->c[7] = (sk->a[3] + sk->b[4] + sk->c[6] - sk->a[4] - sk->b[5] + 192) % 96;
    sk->c[4] = (sk->c[2] + sk->c[5] - sk->c[3] + delta + 96) % 96;
}

const unsigned char *skey_parse(struct skey_t *sk, const unsigned char *s)
{
    unsigned int i;
    memset(sk, 0, sizeof(struct skey_t));
    for (i=0; i<4; i++)
    {
        s = fq_matrix_2x2_parse(&sk->L1_inv[i][0][0], s);
        if (fq_matrix_2x2_inv(&sk->L1[i][0][0], &sk->L1_inv[i][0][0]))
            return NULL;
        s = fq_matrix_2x2_parse(&sk->L2_inv[i][0][0], s);
        if (fq_matrix_2x2_inv(&sk->L2[i][0][0], &sk->L2_inv[i][0][0]))
            return NULL;
        s = fq_matrix_2x2_parse(&sk->L3_inv[i][0][0], s);
        if (fq_matrix_2x2_inv(&sk->L3[i][0][0], &sk->L3_inv[i][0][0]))
            return NULL;
        s = fq_matrix_2x2_parse(&sk->L4_inv[i][0][0], s);
        if (fq_matrix_2x2_inv(&sk->L4[i][0][0], &sk->L4_inv[i][0][0]))
            return NULL;
    }
    for (i=0; i<4; i++)
    {
        s = fq2_parse(sk->A2[i], s);
        s = fq2_parse(sk->A3[i], s);
        s = fq2_parse(sk->A4[i], s);
    }
    if ((sk->a[0] = *s++) >= 96) return NULL;
    if ((sk->a[1] = *s++) >= 96) return NULL;
    if ((sk->a[2] = *s++) >= 96) return NULL;
    if ((sk->a[3] = *s++) >= 96) return NULL;
    if ((sk->a[4] = *s++) >= 96) return NULL;
    if ((sk->a[5] = *s++) >= 96) return NULL;
    if ((sk->b[0] = *s++) >= 96) return NULL;
    if ((sk->b[1] = *s++) >= 96) return NULL;
    if ((sk->b[2] = *s++) >= 96) return NULL;
    if ((sk->b[3] = *s++) >= 96) return NULL;
    if ((sk->b[4] = *s++) >= 96) return NULL;
    if ((sk->b[5] = *s++) >= 96) return NULL;
    if ((sk->c[0] = *s++) >= 96) return NULL;
    if ((sk->c[2] = *s++) >= 96) return NULL;
    if ((sk->c[3] = *s++) >= 96) return NULL;
    if ((sk->c[5] = *s++) >= 96) return NULL;
    if ((sk->c[6] = *s++) >= 96) return NULL;
    sk->c[1] = (sk->a[0] + sk->b[0] + sk->c[0] - sk->a[1] - sk->b[2] + 192) % 96;
    sk->c[7] = (sk->a[3] + sk->b[4] + sk->c[6] - sk->a[4] - sk->b[5] + 192) % 96;
    sk->c[4] = (sk->c[2] + sk->c[5] - sk->c[3] + delta + 96) % 96;
    return s;
}

unsigned char *skey_serialize(unsigned char *s, const struct skey_t *sk)
{
    unsigned int i;
    for (i=0; i<4; i++)
    {
        s = fq_matrix_2x2_serialize(s, &sk->L1_inv[i][0][0]);
        s = fq_matrix_2x2_serialize(s, &sk->L2_inv[i][0][0]);
        s = fq_matrix_2x2_serialize(s, &sk->L3_inv[i][0][0]);
        s = fq_matrix_2x2_serialize(s, &sk->L4_inv[i][0][0]);
    }
    for (i=0; i<4; i++)
    {
        s = fq2_serialize(s, sk->A2[i]);
        s = fq2_serialize(s, sk->A3[i]);
        s = fq2_serialize(s, sk->A4[i]);
    }
    *s++ = sk->a[0];
    *s++ = sk->a[1];
    *s++ = sk->a[2];
    *s++ = sk->a[3];
    *s++ = sk->a[4];
    *s++ = sk->a[5];
    *s++ = sk->b[0];
    *s++ = sk->b[1];
    *s++ = sk->b[2];
    *s++ = sk->b[3];
    *s++ = sk->b[4];
    *s++ = sk->b[5];
    *s++ = sk->c[0];
    *s++ = sk->c[2];
    *s++ = sk->c[3];
    *s++ = sk->c[5];
    *s++ = sk->c[6];
    return s;
}

void apply_expn_1(fq_elem *x, const unsigned char *a, const fq_elem *y)
{
    fq2_elem tmp1, tmp2;
    fq2_pow_2_k(&x[0], &y[0], a[0]);
    fq2_pow_2_k(tmp1, &y[0], a[1]);
    fq2_pow_2_k(tmp2, &y[2], a[2]);
    fq2_mul(&x[2], tmp1, tmp2);
    fq2_pow_2_k(&x[4], &y[4], a[3]);
    fq2_pow_2_k(tmp1, &y[4], a[4]);
    fq2_pow_2_k(tmp2, &y[6], a[5]);
    fq2_mul(&x[6], tmp1, tmp2);
}

void apply_expn_2(fq_elem *x, const unsigned char *b, const fq_elem *y)
{
    fq2_elem tmp1, tmp2;
    fq2_pow_2_k(tmp1, &y[0], b[0]);
    fq2_pow_2_k(tmp2, &y[6], b[1]);
    fq2_mul(&x[0], tmp1, tmp2);
    fq2_pow_2_k(&x[2], &y[2], b[2]);
    fq2_pow_2_k(tmp1, &y[2], b[3]);
    fq2_pow_2_k(tmp2, &y[4], b[4]);
    fq2_mul(&x[4], tmp1, tmp2);
    fq2_pow_2_k(&x[6], &y[6], b[5]);
}

void apply_expn_3(fq_elem *x, const unsigned char *c, const fq_elem *y)
{
    fq2_elem tmp1, tmp2;
    fq2_pow_2_k(tmp1, &y[0], c[0]);
    fq2_pow_2_k(tmp2, &y[2], c[1]);
    fq2_mul(&x[0], tmp1, tmp2);
    fq2_pow_2_k(tmp1, &y[2], c[2]);
    fq2_pow_2_k(tmp2, &y[6], c[3]);
    fq2_mul(&x[2], tmp1, tmp2);
    fq2_pow_2_k(tmp1, &y[2], c[4]);
    fq2_pow_2_k(tmp2, &y[6], c[5]);
    fq2_mul(&x[4], tmp1, tmp2);
    fq2_pow_2_k(tmp1, &y[4], c[6]);
    fq2_pow_2_k(tmp2, &y[6], c[7]);
    fq2_mul(&x[6], tmp1, tmp2);
}

void encrypt_with_skey(fq_elem *ct, const struct skey_t *sk, const fq_elem *pt)
{
    unsigned int i;
    fq_elem t1[8], t2[8];
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t1[2*i], &sk->L1[i][0][0], &pt[2*i]);
    apply_expn_1(t2, sk->a, t1);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t1[2*i], &sk->L2[i][0][0], &t2[2*i]);
    for (i=0; i<4; i++)
        fq2_add(&t1[2*i], &t1[2*i], sk->A2[i]);
    apply_expn_2(t2, sk->b, t1);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t1[2*i], &sk->L3[i][0][0], &t2[2*i]);
    for (i=0; i<4; i++)
        fq2_add(&t1[2*i], &t1[2*i], sk->A3[i]);
    apply_expn_3(t2, sk->c, t1);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&ct[2*i], &sk->L4[i][0][0], &t2[2*i]);
    for (i=0; i<4; i++)
        fq2_add(&ct[2*i], &ct[2*i], sk->A4[i]);
}

void apply_expn_inv_1(fq_elem *x, const unsigned char *a, const fq_elem *y)
{
    fq2_elem tmp1, tmp2, inv1, inv2;
    fq2_inv(inv1, &y[0]);
    fq2_pow_2_k(&x[0], &y[0], (96-a[0]) % 96);
    fq2_pow_2_k(tmp1,  inv1, (192+a[1]-a[0]-a[2]) % 96);
    fq2_pow_2_k(tmp2,  &y[2], (96-a[2]) % 96);
    fq2_mul(&x[2], tmp1, tmp2);
    fq2_inv(inv2, &y[4]);
    fq2_pow_2_k(&x[4], &y[4], (96-a[3]) % 96);
    fq2_pow_2_k(tmp1,  inv2, (192+a[4]-a[3]-a[5]) % 96);
    fq2_pow_2_k(tmp2,  &y[6], (96-a[5]) % 96);
    fq2_mul(&x[6], tmp1, tmp2);
}

void apply_expn_inv_2(fq_elem *x, const unsigned char *b, const fq_elem *y)
{
    fq2_elem tmp1, tmp2, inv1, inv2;
    fq2_inv(inv1, &y[6]);
    fq2_pow_2_k(tmp1, &y[0], (96-b[0]) % 96);
    fq2_pow_2_k(tmp2, inv1, (192+b[1]-b[0]-b[5]) % 96);
    fq2_mul(&x[0], tmp1, tmp2);
    fq2_pow_2_k(&x[2], &y[2], (96-b[2]) % 96);
    fq2_inv(inv2, &y[2]);
    fq2_pow_2_k(tmp1, &y[4], (96-b[4]) % 96);
    fq2_pow_2_k(tmp2, inv2, (192+b[3]-b[4]-b[2]) % 96);
    fq2_mul(&x[4], tmp1, tmp2);
    fq2_pow_2_k(&x[6], &y[6], (96-b[5]) % 96);
}

void apply_expn_inv_3(fq_elem *x, const unsigned char *c, const fq_elem *y)
{
    fq2_elem tmp1, tmp2, inv1, inv2;
    fq2_inv(inv1, &y[2]);
    fq2_inv(inv2, &y[4]);
    fq2_pow_2_k(tmp1, inv2,  (192+c[1]-c[0]-c[4]+delta) % 96);
    fq2_pow_2_k(tmp2, &y[2], (192+c[1]-c[0]-c[2]) % 96);
    fq2_mul(tmp1, tmp1, tmp2);
    fq2_pow(tmp1, tmp1, fq2_inv_delta);
    fq2_pow_2_k(tmp2, &y[0], (96-c[0]) % 96);
    fq2_mul(&x[0], tmp1, tmp2);
    fq2_pow_2_k(tmp1, inv1,  (96-c[2]) % 96);
    fq2_pow_2_k(tmp2, &y[4], (192+c[3]-c[2]-c[5]) % 96);
    fq2_mul(tmp1, tmp1, tmp2);
    fq2_pow(&x[2], tmp1, fq2_inv_delta);
    fq2_pow_2_k(tmp1, &y[4], (192+c[7]-c[6]-c[5]) % 96);
    fq2_pow_2_k(tmp2, inv1,  (192+c[7]-c[6]-c[3]+delta) % 96);
    fq2_mul(tmp1, tmp1, tmp2);
    fq2_pow(tmp1, tmp1, fq2_inv_delta);
    fq2_pow_2_k(tmp2, &y[6], (96-c[6]) % 96);
    fq2_mul(&x[4], tmp1, tmp2);
    fq2_pow_2_k(tmp1, &y[2], (96+delta-c[3]) % 96);
    fq2_pow_2_k(tmp2, inv2, (96-c[5]) % 96);
    fq2_mul(tmp1, tmp1, tmp2);
    fq2_pow(&x[6], tmp1, fq2_inv_delta);
}

void decrypt_with_skey(fq_elem *pt, const struct skey_t *sk, const fq_elem *ct)
{
    unsigned int i;
    fq_elem t1[8], t2[8];
    for (i=0; i<4; i++)
        fq2_add(&t1[2*i], &ct[2*i], sk->A4[i]);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t2[2*i], &sk->L4_inv[i][0][0], &t1[2*i]);
    apply_expn_inv_3(t1, sk->c, t2);
    for (i=0; i<4; i++)
        fq2_add(&t1[2*i], &t1[2*i], sk->A3[i]);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t2[2*i], &sk->L3_inv[i][0][0], &t1[2*i]);
    apply_expn_inv_2(t1, sk->b, t2);
    for (i=0; i<4; i++)
        fq2_add(&t1[2*i], &t1[2*i], sk->A2[i]);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&t2[2*i], &sk->L2_inv[i][0][0], &t1[2*i]);
    apply_expn_inv_1(t1, sk->a, t2);
    for (i=0; i<4; i++)
        fq_matrix_2x2_map(&pt[2*i], &sk->L1_inv[i][0][0], &t1[2*i]);
}

void fq2_matrix_2x2_map_polynomial(fq_elem *p, const fq_elem *m, const fq_elem *q,
    unsigned int n)
{
    unsigned int i;
    for (i=0; i<n; i++)
        fq_matrix_2x2_map(&p[2*i], m, &q[2*i]);
}

void fq2_polynomial_product(fq_elem *p,
    const fq_elem *q1, unsigned int a1, unsigned int n1,
    const fq_elem *q2, unsigned int a2, unsigned int n2)
{
    unsigned int i, j, k;
    fq2_elem tmp1, tmp2;
    for (i=k=0; i<n1; i++)
        for (j=0; j<n2; j++, k++)
        {
            fq2_pow_2_k(tmp1, &q1[2*i], a1);
            fq2_pow_2_k(tmp2, &q2[2*j], a2);
            fq2_mul(&p[2*k], tmp1, tmp2);
        }
}

void fq2_polynomial_pow_2_k(fq_elem *p,
    const fq_elem *q, unsigned int k, unsigned int n)
{
    unsigned int i;
    for (i=0; i<n; i++)
        fq2_pow_2_k(&p[2*i], &q[2*i], k);
}

const unsigned int p12_4_red_idx[75] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 2, 3, 25, 26, 27, 7, 8, 28, 29, 30, 12, 13, 31, 32, 33, 17, 18, 34, 35, 36, 22, 23, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64
};

const unsigned int p34_4_red_idx[25] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
};

const unsigned int p56_4_red_idx[25] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
};

const unsigned int p78_4_red_idx[75] =
{
    0, 1, 2, 3, 4, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 15, 16, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 28, 29, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 41, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64
};

void generate_pkey(struct pkey_t *pk, struct skey_t *sk)
{
    unsigned int i;
    fq2_elem p12_1[2],  p34_1[2], p56_1[2], p78_1[2];
    fq2_elem p12_2[3],  p34_2[5], p56_2[3], p78_2[5];
    fq2_elem p12_3[15], p34_3[5], p56_3[15], p78_3[5];
    fq2_elem p12_4[75], p34_4[25], p56_4[25], p78_4[75];
    memset(pk, 0, sizeof(struct pkey_t));
    fq2_set(p12_1[0], fq2_1);
    fq2_set(p12_1[1], fq2_u);
    fq2_set(p34_1[0], fq2_1);
    fq2_set(p34_1[1], fq2_u);
    fq2_set(p56_1[0], fq2_1);
    fq2_set(p56_1[1], fq2_u);
    fq2_set(p78_1[0], fq2_1);
    fq2_set(p78_1[1], fq2_u);
    fq2_matrix_2x2_map_polynomial(p12_1[0], &sk->L1[0][0][0], p12_1[0], 2);
    fq2_matrix_2x2_map_polynomial(p34_1[0], &sk->L1[1][0][0], p34_1[0], 2);
    fq2_matrix_2x2_map_polynomial(p56_1[0], &sk->L1[2][0][0], p56_1[0], 2);
    fq2_matrix_2x2_map_polynomial(p78_1[0], &sk->L1[3][0][0], p78_1[0], 2);
    fq2_polynomial_pow_2_k(p12_2[0], p12_1[0], sk->a[0], 2);
    fq2_polynomial_product(p34_2[0], p12_1[0], sk->a[1], 2, p34_1[0], sk->a[2], 2);
    fq2_polynomial_pow_2_k(p56_2[0], p56_1[0], sk->a[3], 2);
    fq2_polynomial_product(p78_2[0], p56_1[0], sk->a[4], 2, p78_1[0], sk->a[5], 2);
    fq2_matrix_2x2_map_polynomial(p12_2[0], &sk->L2[0][0][0], p12_2[0], 2);
    fq2_matrix_2x2_map_polynomial(p34_2[0], &sk->L2[1][0][0], p34_2[0], 4);
    fq2_matrix_2x2_map_polynomial(p56_2[0], &sk->L2[2][0][0], p56_2[0], 2);
    fq2_matrix_2x2_map_polynomial(p78_2[0], &sk->L2[3][0][0], p78_2[0], 4);
    fq2_set(p12_2[2], sk->A2[0]);
    fq2_set(p34_2[4], sk->A2[1]);
    fq2_set(p56_2[2], sk->A2[2]);
    fq2_set(p78_2[4], sk->A2[3]);
    fq2_polynomial_product(p12_3[0], p12_2[0], sk->b[0], 3, p78_2[0], sk->b[1], 5);
    fq2_polynomial_pow_2_k(p34_3[0], p34_2[0], sk->b[2], 5);
    fq2_polynomial_product(p56_3[0], p34_2[0], sk->b[3], 5, p56_2[0], sk->b[4], 3);
    fq2_polynomial_pow_2_k(p78_3[0], p78_2[0], sk->b[5], 5);
    fq2_matrix_2x2_map_polynomial(p12_3[0], &sk->L3[0][0][0], p12_3[0], 15);
    fq2_matrix_2x2_map_polynomial(p34_3[0], &sk->L3[1][0][0], p34_3[0], 5);
    fq2_matrix_2x2_map_polynomial(p56_3[0], &sk->L3[2][0][0], p56_3[0], 15);
    fq2_matrix_2x2_map_polynomial(p78_3[0], &sk->L3[3][0][0], p78_3[0], 5);
    fq2_add(p12_3[14], p12_3[14], sk->A3[0]);
    fq2_add(p34_3[4],  p34_3[4],  sk->A3[1]);
    fq2_add(p56_3[14], p56_3[14], sk->A3[2]);
    fq2_add(p78_3[4],  p78_3[4],  sk->A3[3]);
    fq2_polynomial_product(p12_4[0], p12_3[0], sk->c[0], 15, p34_3[0], sk->c[1], 5);
    fq2_polynomial_product(p34_4[0], p34_3[0], sk->c[2], 5,  p78_3[0], sk->c[3], 5);
    fq2_polynomial_product(p56_4[0], p34_3[0], sk->c[4], 5,  p78_3[0], sk->c[5], 5);
    fq2_polynomial_product(p78_4[0], p56_3[0], sk->c[6], 15,  p78_3[0], sk->c[7], 5);
    fq2_matrix_2x2_map_polynomial(p12_4[0], &sk->L4[0][0][0], p12_4[0], 75);
    fq2_matrix_2x2_map_polynomial(p34_4[0], &sk->L4[1][0][0], p34_4[0], 25);
    fq2_matrix_2x2_map_polynomial(p56_4[0], &sk->L4[2][0][0], p56_4[0], 25);
    fq2_matrix_2x2_map_polynomial(p78_4[0], &sk->L4[3][0][0], p78_4[0], 75);
    fq2_add(p12_4[74], p12_4[74], sk->A4[0]);
    fq2_add(p34_4[24], p34_4[24], sk->A4[1]);
    fq2_add(p56_4[24], p56_4[24], sk->A4[2]);
    fq2_add(p78_4[74], p78_4[74], sk->A4[3]);
    for (i=0; i<75; i++)
        fq2_add(pk->p12[p12_4_red_idx[i]], pk->p12[p12_4_red_idx[i]], p12_4[i]);
    for (i=0; i<25; i++)
        fq2_add(pk->p34[p34_4_red_idx[i]], pk->p34[p34_4_red_idx[i]], p34_4[i]);
    for (i=0; i<25; i++)
        fq2_add(pk->p56[p56_4_red_idx[i]], pk->p56[p56_4_red_idx[i]], p56_4[i]);
    for (i=0; i<75; i++)
        fq2_add(pk->p78[p78_4_red_idx[i]], pk->p78[p78_4_red_idx[i]], p78_4[i]);
    /* x1, x2 */
    pk->f[0]  = (sk->a[0] + sk->b[0] + sk->c[0]) % 48;
    pk->f[1]  = (sk->a[1] + sk->b[2] + sk->c[2]) % 48;
    pk->f[2]  = (sk->a[1] + sk->b[2] + sk->c[4]) % 48;
    pk->f[3]  = (sk->a[1] + sk->b[3] + sk->c[6]) % 48;
    /* x3, x4 */
    pk->f[4]  = (sk->a[2] + sk->a[0] + sk->b[0] - sk->a[1] + sk->c[0] + 96) % 48;
    pk->f[5]  = (sk->a[2] + sk->b[2] + sk->c[2]) % 48;
    pk->f[6]  = (sk->a[2] + sk->b[2] + sk->c[4]) % 48;
    pk->f[7]  = (sk->a[2] + sk->b[3] + sk->c[6]) % 48;
    /* x5, x6 */
    pk->f[8]  = (sk->a[4] + sk->b[1] + sk->c[0]) % 48;
    pk->f[9]  = (sk->a[4] + sk->b[5] + sk->c[3]) % 48;
    pk->f[10] = (sk->a[4] + sk->b[5] + sk->c[5]) % 48;
    pk->f[11] = (sk->a[3] + sk->b[4] + sk->c[6]) % 48;
    /* x7, x8 */
    pk->f[12] = (sk->a[5] + sk->b[1] + sk->c[0]) % 48;
    pk->f[13] = (sk->a[5] + sk->b[5] + sk->c[3]) % 48;
    pk->f[14] = (sk->a[5] + sk->b[5] + sk->c[5]) % 48;
    pk->f[15] = (sk->a[5] + sk->a[3] + sk->b[4] - sk->a[4] + sk->c[6] + 96) % 48;
}

fq_elem fq_monomio(unsigned int n, ...)
{
    unsigned int i;
    fq_elem x, y;
    va_list ap;
    if (!n) return fq_1;
    va_start(ap, n);
    x = va_arg(ap, fq_elem);
    for (i=1; i<n; i++)
    {
        y = va_arg(ap, fq_elem);
        x = fq_mul(x, y);
    }
    va_end(ap);
    return x;
}

void encrypt_with_pkey(fq_elem *ct, const struct pkey_t *pk, const fq_elem *pt)
{
    unsigned int i;
    fq_elem y1[32], y2[32], c12[65], c34[25], c56[25], c78[65];
    for (i=0; i<4; i++)
    {
        y1[i]    = fq_pow_2_k(pt[0], pk->f[i]);
        y1[i+4]  = fq_pow_2_k(pt[1], pk->f[i]);
        y1[i+8]  = fq_pow_2_k(pt[2], pk->f[i+4]);
        y1[i+12] = fq_pow_2_k(pt[3], pk->f[i+4]);
        y1[i+16] = fq_pow_2_k(pt[4], pk->f[i+8]);
        y1[i+20] = fq_pow_2_k(pt[5], pk->f[i+8]);
        y1[i+24] = fq_pow_2_k(pt[6], pk->f[i+12]);
        y1[i+28] = fq_pow_2_k(pt[7], pk->f[i+12]);
    }    
    for (i=0; i<32; i++)
        y2[i] = fq_mul(y1[i], y1[i]);
    c12[0] = fq_monomio(4,y1[8], y2[0], y1[24], y1[16]);
    c12[1] = fq_monomio(4,y1[12], y2[0], y1[24], y1[16]);
    c12[2] = fq_monomio(5,y1[8], y1[4], y1[24], y1[16], y1[0]);
    c12[3] = fq_monomio(5,y1[12], y1[4], y1[24], y1[16], y1[0]);
    c12[4] = fq_monomio(3,y1[24], y1[16], y1[0]);
    c12[5] = fq_monomio(4,y1[8], y2[0], y1[28], y1[16]);
    c12[6] = fq_monomio(4,y1[12], y2[0], y1[28], y1[16]);
    c12[7] = fq_monomio(5,y1[8], y1[4], y1[28], y1[16], y1[0]);
    c12[8] = fq_monomio(5,y1[12], y1[4], y1[28], y1[16], y1[0]);
    c12[9] = fq_monomio(3,y1[28], y1[16], y1[0]);
    c12[10] = fq_monomio(4,y1[8], y2[0], y1[24], y1[20]);
    c12[11] = fq_monomio(4,y1[12], y2[0], y1[24], y1[20]);
    c12[12] = fq_monomio(5,y1[8], y1[4], y1[24], y1[20], y1[0]);
    c12[13] = fq_monomio(5,y1[12], y1[4], y1[24], y1[20], y1[0]);
    c12[14] = fq_monomio(3,y1[24], y1[20], y1[0]);
    c12[15] = fq_monomio(4,y1[8], y2[0], y1[28], y1[20]);
    c12[16] = fq_monomio(4,y1[12], y2[0], y1[28], y1[20]);
    c12[17] = fq_monomio(5,y1[8], y1[4], y1[28], y1[20], y1[0]);
    c12[18] = fq_monomio(5,y1[12], y1[4], y1[28], y1[20], y1[0]);
    c12[19] = fq_monomio(3,y1[28], y1[20], y1[0]);
    c12[20] = fq_monomio(2,y1[8], y2[0]);
    c12[21] = fq_monomio(2,y1[12], y2[0]);
    c12[22] = fq_monomio(3,y1[8], y1[4], y1[0]);
    c12[23] = fq_monomio(3,y1[12], y1[4], y1[0]);
    c12[24] = fq_monomio(1,y1[0]);
    c12[25] = fq_monomio(4,y1[8], y2[4], y1[24], y1[16]);
    c12[26] = fq_monomio(4,y1[12], y2[4], y1[24], y1[16]);
    c12[27] = fq_monomio(3,y1[24], y1[16], y1[4]);
    c12[28] = fq_monomio(4,y1[8], y2[4], y1[28], y1[16]);
    c12[29] = fq_monomio(4,y1[12], y2[4], y1[28], y1[16]);
    c12[30] = fq_monomio(3,y1[28], y1[16], y1[4]);
    c12[31] = fq_monomio(4,y1[8], y2[4], y1[24], y1[20]);
    c12[32] = fq_monomio(4,y1[12], y2[4], y1[24], y1[20]);
    c12[33] = fq_monomio(3,y1[24], y1[20], y1[4]);
    c12[34] = fq_monomio(4,y1[8], y2[4], y1[28], y1[20]);
    c12[35] = fq_monomio(4,y1[12], y2[4], y1[28], y1[20]);
    c12[36] = fq_monomio(3,y1[28], y1[20], y1[4]);
    c12[37] = fq_monomio(2,y1[8], y2[4]);
    c12[38] = fq_monomio(2,y1[12], y2[4]);
    c12[39] = fq_monomio(1,y1[4]);
    c12[40] = fq_monomio(4,y1[8], y1[0], y1[24], y1[16]);
    c12[41] = fq_monomio(4,y1[12], y1[0], y1[24], y1[16]);
    c12[42] = fq_monomio(4,y1[8], y1[4], y1[24], y1[16]);
    c12[43] = fq_monomio(4,y1[12], y1[4], y1[24], y1[16]);
    c12[44] = fq_monomio(2,y1[24], y1[16]);
    c12[45] = fq_monomio(4,y1[8], y1[0], y1[28], y1[16]);
    c12[46] = fq_monomio(4,y1[12], y1[0], y1[28], y1[16]);
    c12[47] = fq_monomio(4,y1[8], y1[4], y1[28], y1[16]);
    c12[48] = fq_monomio(4,y1[12], y1[4], y1[28], y1[16]);
    c12[49] = fq_monomio(2,y1[28], y1[16]);
    c12[50] = fq_monomio(4,y1[8], y1[0], y1[24], y1[20]);
    c12[51] = fq_monomio(4,y1[12], y1[0], y1[24], y1[20]);
    c12[52] = fq_monomio(4,y1[8], y1[4], y1[24], y1[20]);
    c12[53] = fq_monomio(4,y1[12], y1[4], y1[24], y1[20]);
    c12[54] = fq_monomio(2,y1[24], y1[20]);
    c12[55] = fq_monomio(4,y1[8], y1[0], y1[28], y1[20]);
    c12[56] = fq_monomio(4,y1[12], y1[0], y1[28], y1[20]);
    c12[57] = fq_monomio(4,y1[8], y1[4], y1[28], y1[20]);
    c12[58] = fq_monomio(4,y1[12], y1[4], y1[28], y1[20]);
    c12[59] = fq_monomio(2,y1[28], y1[20]);
    c12[60] = fq_monomio(2,y1[8], y1[0]);
    c12[61] = fq_monomio(2,y1[12], y1[0]);
    c12[62] = fq_monomio(2,y1[8], y1[4]);
    c12[63] = fq_monomio(2,y1[12], y1[4]);
    c12[64] = fq_1;
    c34[0] = fq_monomio(4,y1[25], y1[17], y1[9], y1[1]);
    c34[1] = fq_monomio(4,y1[29], y1[17], y1[9], y1[1]);
    c34[2] = fq_monomio(4,y1[25], y1[21], y1[9], y1[1]);
    c34[3] = fq_monomio(4,y1[29], y1[21], y1[9], y1[1]);
    c34[4] = fq_monomio(2,y1[9], y1[1]);
    c34[5] = fq_monomio(4,y1[25], y1[17], y1[13], y1[1]);
    c34[6] = fq_monomio(4,y1[29], y1[17], y1[13], y1[1]);
    c34[7] = fq_monomio(4,y1[25], y1[21], y1[13], y1[1]);
    c34[8] = fq_monomio(4,y1[29], y1[21], y1[13], y1[1]);
    c34[9] = fq_monomio(2,y1[13], y1[1]);
    c34[10] = fq_monomio(4,y1[25], y1[17], y1[9], y1[5]);
    c34[11] = fq_monomio(4,y1[29], y1[17], y1[9], y1[5]);
    c34[12] = fq_monomio(4,y1[25], y1[21], y1[9], y1[5]);
    c34[13] = fq_monomio(4,y1[29], y1[21], y1[9], y1[5]);
    c34[14] = fq_monomio(2,y1[9], y1[5]);
    c34[15] = fq_monomio(4,y1[25], y1[17], y1[13], y1[5]);
    c34[16] = fq_monomio(4,y1[29], y1[17], y1[13], y1[5]);
    c34[17] = fq_monomio(4,y1[25], y1[21], y1[13], y1[5]);
    c34[18] = fq_monomio(4,y1[29], y1[21], y1[13], y1[5]);
    c34[19] = fq_monomio(2,y1[13], y1[5]);
    c34[20] = fq_monomio(2,y1[25], y1[17]);
    c34[21] = fq_monomio(2,y1[29], y1[17]);
    c34[22] = fq_monomio(2,y1[25], y1[21]);
    c34[23] = fq_monomio(2,y1[29], y1[21]);
    c34[24] = fq_1;
    c56[0] = fq_monomio(4,y1[26], y1[18], y1[10], y1[2]);
    c56[1] = fq_monomio(4,y1[30], y1[18], y1[10], y1[2]);
    c56[2] = fq_monomio(4,y1[26], y1[22], y1[10], y1[2]);
    c56[3] = fq_monomio(4,y1[30], y1[22], y1[10], y1[2]);
    c56[4] = fq_monomio(2,y1[10], y1[2]);
    c56[5] = fq_monomio(4,y1[26], y1[18], y1[14], y1[2]);
    c56[6] = fq_monomio(4,y1[30], y1[18], y1[14], y1[2]);
    c56[7] = fq_monomio(4,y1[26], y1[22], y1[14], y1[2]);
    c56[8] = fq_monomio(4,y1[30], y1[22], y1[14], y1[2]);
    c56[9] = fq_monomio(2,y1[14], y1[2]);
    c56[10] = fq_monomio(4,y1[26], y1[18], y1[10], y1[6]);
    c56[11] = fq_monomio(4,y1[30], y1[18], y1[10], y1[6]);
    c56[12] = fq_monomio(4,y1[26], y1[22], y1[10], y1[6]);
    c56[13] = fq_monomio(4,y1[30], y1[22], y1[10], y1[6]);
    c56[14] = fq_monomio(2,y1[10], y1[6]);
    c56[15] = fq_monomio(4,y1[26], y1[18], y1[14], y1[6]);
    c56[16] = fq_monomio(4,y1[30], y1[18], y1[14], y1[6]);
    c56[17] = fq_monomio(4,y1[26], y1[22], y1[14], y1[6]);
    c56[18] = fq_monomio(4,y1[30], y1[22], y1[14], y1[6]);
    c56[19] = fq_monomio(2,y1[14], y1[6]);
    c56[20] = fq_monomio(2,y1[26], y1[18]);
    c56[21] = fq_monomio(2,y1[30], y1[18]);
    c56[22] = fq_monomio(2,y1[26], y1[22]);
    c56[23] = fq_monomio(2,y1[30], y1[22]);
    c56[24] = fq_1;
    c78[0] = fq_monomio(4,y1[27], y2[19], y1[11], y1[3]);
    c78[1] = fq_monomio(4,y1[31], y2[19], y1[11], y1[3]);
    c78[2] = fq_monomio(5,y1[27], y1[23], y1[19], y1[11], y1[3]);
    c78[3] = fq_monomio(5,y1[31], y1[23], y1[19], y1[11], y1[3]);
    c78[4] = fq_monomio(3,y1[19], y1[11], y1[3]);
    c78[5] = fq_monomio(4,y1[27], y2[23], y1[11], y1[3]);
    c78[6] = fq_monomio(4,y1[31], y2[23], y1[11], y1[3]);
    c78[7] = fq_monomio(3,y1[23], y1[11], y1[3]);
    c78[8] = fq_monomio(4,y1[27], y1[19], y1[11], y1[3]);
    c78[9] = fq_monomio(4,y1[31], y1[19], y1[11], y1[3]);
    c78[10] = fq_monomio(4,y1[27], y1[23], y1[11], y1[3]);
    c78[11] = fq_monomio(4,y1[31], y1[23], y1[11], y1[3]);
    c78[12] = fq_monomio(2,y1[11], y1[3]);
    c78[13] = fq_monomio(4,y1[27], y2[19], y1[15], y1[3]);
    c78[14] = fq_monomio(4,y1[31], y2[19], y1[15], y1[3]);
    c78[15] = fq_monomio(5,y1[27], y1[23], y1[19], y1[15], y1[3]);
    c78[16] = fq_monomio(5,y1[31], y1[23], y1[19], y1[15], y1[3]);
    c78[17] = fq_monomio(3,y1[19], y1[15], y1[3]);
    c78[18] = fq_monomio(4,y1[27], y2[23], y1[15], y1[3]);
    c78[19] = fq_monomio(4,y1[31], y2[23], y1[15], y1[3]);
    c78[20] = fq_monomio(3,y1[23], y1[15], y1[3]);
    c78[21] = fq_monomio(4,y1[27], y1[19], y1[15], y1[3]);
    c78[22] = fq_monomio(4,y1[31], y1[19], y1[15], y1[3]);
    c78[23] = fq_monomio(4,y1[27], y1[23], y1[15], y1[3]);
    c78[24] = fq_monomio(4,y1[31], y1[23], y1[15], y1[3]);
    c78[25] = fq_monomio(2,y1[15], y1[3]);
    c78[26] = fq_monomio(4,y1[27], y2[19], y1[11], y1[7]);
    c78[27] = fq_monomio(4,y1[31], y2[19], y1[11], y1[7]);
    c78[28] = fq_monomio(5,y1[27], y1[23], y1[19], y1[11], y1[7]);
    c78[29] = fq_monomio(5,y1[31], y1[23], y1[19], y1[11], y1[7]);
    c78[30] = fq_monomio(3,y1[19], y1[11], y1[7]);
    c78[31] = fq_monomio(4,y1[27], y2[23], y1[11], y1[7]);
    c78[32] = fq_monomio(4,y1[31], y2[23], y1[11], y1[7]);
    c78[33] = fq_monomio(3,y1[23], y1[11], y1[7]);
    c78[34] = fq_monomio(4,y1[27], y1[19], y1[11], y1[7]);
    c78[35] = fq_monomio(4,y1[31], y1[19], y1[11], y1[7]);
    c78[36] = fq_monomio(4,y1[27], y1[23], y1[11], y1[7]);
    c78[37] = fq_monomio(4,y1[31], y1[23], y1[11], y1[7]);
    c78[38] = fq_monomio(2,y1[11], y1[7]);
    c78[39] = fq_monomio(4,y1[27], y2[19], y1[15], y1[7]);
    c78[40] = fq_monomio(4,y1[31], y2[19], y1[15], y1[7]);
    c78[41] = fq_monomio(5,y1[27], y1[23], y1[19], y1[15], y1[7]);
    c78[42] = fq_monomio(5,y1[31], y1[23], y1[19], y1[15], y1[7]);
    c78[43] = fq_monomio(3,y1[19], y1[15], y1[7]);
    c78[44] = fq_monomio(4,y1[27], y2[23], y1[15], y1[7]);
    c78[45] = fq_monomio(4,y1[31], y2[23], y1[15], y1[7]);
    c78[46] = fq_monomio(3,y1[23], y1[15], y1[7]);
    c78[47] = fq_monomio(4,y1[27], y1[19], y1[15], y1[7]);
    c78[48] = fq_monomio(4,y1[31], y1[19], y1[15], y1[7]);
    c78[49] = fq_monomio(4,y1[27], y1[23], y1[15], y1[7]);
    c78[50] = fq_monomio(4,y1[31], y1[23], y1[15], y1[7]);
    c78[51] = fq_monomio(2,y1[15], y1[7]);
    c78[52] = fq_monomio(2,y1[27], y2[19]);
    c78[53] = fq_monomio(2,y1[31], y2[19]);
    c78[54] = fq_monomio(3,y1[27], y1[23], y1[19]);
    c78[55] = fq_monomio(3,y1[31], y1[23], y1[19]);
    c78[56] = fq_monomio(1,y1[19]);
    c78[57] = fq_monomio(2,y1[27], y2[23]);
    c78[58] = fq_monomio(2,y1[31], y2[23]);
    c78[59] = fq_monomio(1,y1[23]);
    c78[60] = fq_monomio(2,y1[27], y1[19]);
    c78[61] = fq_monomio(2,y1[31], y1[19]);
    c78[62] = fq_monomio(2,y1[27], y1[23]);
    c78[63] = fq_monomio(2,y1[31], y1[23]);
    c78[64] = fq_1;
    memset(ct, 0, 8*sizeof(fq_elem));
    for (i=0; i<65; i++)
    {
         ct[0] = fq_add(ct[0], fq_mul(c12[i], pk->p12[i][0]));
         ct[1] = fq_add(ct[1], fq_mul(c12[i], pk->p12[i][1]));
    }
    for (i=0; i<25; i++)
    {
         ct[2] = fq_add(ct[2], fq_mul(c34[i], pk->p34[i][0]));
         ct[3] = fq_add(ct[3], fq_mul(c34[i], pk->p34[i][1]));
    }
    for (i=0; i<25; i++)
    {
         ct[4] = fq_add(ct[4], fq_mul(c56[i], pk->p56[i][0]));
         ct[5] = fq_add(ct[5], fq_mul(c56[i], pk->p56[i][1]));
    }
    for (i=0; i<65; i++)
    {
         ct[6] = fq_add(ct[6], fq_mul(c78[i], pk->p78[i][0]));
         ct[7] = fq_add(ct[7], fq_mul(c78[i], pk->p78[i][1]));
    }
}

const unsigned char *pkey_parse(struct pkey_t *pk, const unsigned char *s)
{
    unsigned int i;
    memset(pk, 0, sizeof(struct pkey_t));
    for (i=0; i<65; i++)
        s = fq2_parse(pk->p12[i], s);
    for (i=0; i<25; i++)
        s = fq2_parse(pk->p34[i], s);
    for (i=0; i<25; i++)
        s = fq2_parse(pk->p56[i], s);
    for (i=0; i<65; i++)
        s = fq2_parse(pk->p78[i], s);
    if ((pk->f[0] = *s++) >= 48) return NULL;
    if ((pk->f[1] = *s++) >= 48) return NULL;
//    if ((pk->f[2] = *s++) >= 48) return NULL;
    if ((pk->f[3] = *s++) >= 48) return NULL;
//    if ((pk->f[4] = *s++) >= 48) return NULL;
    if ((pk->f[5] = *s++) >= 48) return NULL;
//    if ((pk->f[6] = *s++) >= 48) return NULL;
//    if ((pk->f[7] = *s++) >= 48) return NULL;
    if ((pk->f[8] = *s++) >= 48) return NULL;
    if ((pk->f[9] = *s++) >= 48) return NULL;
    if ((pk->f[10] = *s++) >= 48) return NULL;
    if ((pk->f[11] = *s++) >= 48) return NULL;
    if ((pk->f[12] = *s++) >= 48) return NULL;
//    if ((pk->f[13] = *s++) >= 48) return NULL;
//    if ((pk->f[14] = *s++) >= 48) return NULL;
//    if ((pk->f[15] = *s++) >= 48) return NULL;
    pk->f[2]  = (pk->f[1]  + pk->f[10] - pk->f[9] + delta + 48) % 48;
    pk->f[4]  = (pk->f[0]  + pk->f[5]  - pk->f[1] + 48) % 48;
    pk->f[6]  = (pk->f[5]  + pk->f[2]  - pk->f[1] + 48) % 48;
    pk->f[7]  = (pk->f[5]  + pk->f[3]  - pk->f[1] + 48) % 48;
    pk->f[13] = (pk->f[12] + pk->f[9]  - pk->f[8] + 48) % 48;
    pk->f[14] = (pk->f[12] + pk->f[10] - pk->f[8] + 48) % 48;
    pk->f[15] = (pk->f[11] + pk->f[12] - pk->f[8] + 48) % 48;
    return s;
}

unsigned char *pkey_serialize(unsigned char *s, const struct pkey_t *pk)
{
    unsigned int i;
    for (i=0; i<65; i++)
        s = fq2_serialize(s, pk->p12[i]);
    for (i=0; i<25; i++)
        s = fq2_serialize(s, pk->p34[i]);
    for (i=0; i<25; i++)
        s = fq2_serialize(s, pk->p56[i]);
    for (i=0; i<65; i++)
        s = fq2_serialize(s, pk->p78[i]);
    *s++ = pk->f[0];
    *s++ = pk->f[1];
//    *s++ = pk->f[2];
    *s++ = pk->f[3];
//    *s++ = pk->f[4];
    *s++ = pk->f[5];
//    *s++ = pk->f[6];
//    *s++ = pk->f[7];
    *s++ = pk->f[8];
    *s++ = pk->f[9];
    *s++ = pk->f[10];
    *s++ = pk->f[11];
    *s++ = pk->f[12];
//    *s++ = pk->f[13];
//    *s++ = pk->f[14];
//    *s++ = pk->f[15];
    return s;
}
