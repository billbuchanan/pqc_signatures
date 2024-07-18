#ifndef DME_H
#define DME_H

#include <stdint.h>

typedef uint64_t fq_elem;
typedef uint64_t fq_expn;

#define fq_0   UINT64_C(0x000000000000)
#define fq_1   UINT64_C(0x000000000001)
#define fq_t   UINT64_C(0x000000000002)
#define fq_t48 UINT64_C(0x000000020007)  /* Fq = F2[t]/<t^48+t^17+t^2+t+1> */
#define fq_qm1 UINT64_C(0xffffffffffff)
#define fq_qm2 UINT64_C(0xfffffffffffe)

fq_elem fq_add(fq_elem a, fq_elem b);
fq_elem fq_mul(fq_elem a, fq_elem b);
fq_elem fq_sqr(fq_elem a);
fq_elem fq_pow(fq_elem a, fq_expn n);
fq_elem fq_inv(fq_elem a);
fq_elem fq_pow_2_k(fq_elem a, unsigned int k);
fq_elem fq_rnd(void);
void fq_print(fq_elem a);

const unsigned char *fq_parse(fq_elem *x, const unsigned char *s);
unsigned char *fq_serialize(unsigned char *s, fq_elem x);

typedef fq_elem fq2_elem[2];
typedef fq_expn fq2_expn[2];

#define delta 17

extern const fq2_elem fq2_0, fq2_1, fq2_u, fq2_u2;
extern const fq2_expn fq2_q2m1, fq2_q2m2, fq2_inv_delta;

void fq2_set(fq_elem *a, const fq_elem *b);
void fq2_add(fq_elem *a, const fq_elem *b, const fq_elem *c);
void fq2_mul(fq_elem *a, const fq_elem *b, const fq_elem *c);
void fq2_sqr(fq_elem *a, const fq_elem *b);
void fq2_pow(fq_elem *a, const fq_elem *b, const fq_expn *c);
void fq2_inv(fq_elem *a, const fq_elem *b);
void fq2_pow_2_k(fq_elem *a, const fq_elem *b, unsigned int k);
void fq2_rnd(fq_elem *a);
void fq2_print(const fq_elem *a);

const unsigned char *fq2_parse(fq_elem *x, const unsigned char *s);
unsigned char *fq2_serialize(unsigned char *s, const fq_elem *x);

typedef fq_elem fq_matrix_2x2[2][2];

int  fq_matrix_2x2_inv(fq_elem *inv, const fq_elem *mat);
void fq_matrix_2x2_map(fq_elem *x, const fq_elem *m, const fq_elem *y);
void fq_matrix_2x2_rnd(fq_elem *mat, fq_elem *inv);

const unsigned char *fq_matrix_2x2_parse(fq_elem *m, const unsigned char *s);
unsigned char *fq_matrix_2x2_serialize(unsigned char *s, const fq_elem *m);

struct skey_t
{
    fq_matrix_2x2 L1[4], L1_inv[4];
    fq_matrix_2x2 L2[4], L2_inv[4];
    fq_matrix_2x2 L3[4], L3_inv[4];
    fq_matrix_2x2 L4[4], L4_inv[4];
    fq2_elem A2[4], A3[4], A4[4];
    unsigned char a[6], b[6], c[8];
};

void generate_skey(struct skey_t *sk);

const unsigned char *skey_parse(struct skey_t *sk, const unsigned char *s);
unsigned char *skey_serialize(unsigned char *s, const struct skey_t *sk);

void apply_expn_1(fq_elem *x, const unsigned char *a, const fq_elem *y);
void apply_expn_2(fq_elem *x, const unsigned char *b, const fq_elem *y);
void apply_expn_3(fq_elem *x, const unsigned char *c, const fq_elem *y);

void encrypt_with_skey(fq_elem *ct, const struct skey_t *sk, const fq_elem *pt);

void apply_expn_inv_1(fq_elem *x, const unsigned char *a, const fq_elem *y);
void apply_expn_inv_2(fq_elem *x, const unsigned char *b, const fq_elem *y);
void apply_expn_inv_3(fq_elem *x, const unsigned char *c, const fq_elem *y);

void decrypt_with_skey(fq_elem *pt, const struct skey_t *sk, const fq_elem *ct);

struct pkey_t
{
    fq2_elem p12[65];
    fq2_elem p34[25];
    fq2_elem p56[25];
    fq2_elem p78[65];
    unsigned char f[16];
};

void fq2_matrix_2x2_map_polynomial(fq_elem *p, const fq_elem *m, const fq_elem *q,
    unsigned int n);

void fq2_polynomial_product(fq_elem *p,
    const fq_elem *q1, unsigned int a1, unsigned int n1,
    const fq_elem *q2, unsigned int a2, unsigned int n2);

void fq2_polynomial_pow_2_k(fq_elem *p,
    const fq_elem *q, unsigned int k, unsigned int n);

void generate_pkey(struct pkey_t *pk, struct skey_t *sk);

void encrypt_with_pkey(fq_elem *ct, const struct pkey_t *pk, const fq_elem *pt);

const unsigned char *pkey_parse(struct pkey_t *pk, const unsigned char *s);
unsigned char *pkey_serialize(unsigned char *s, const struct pkey_t *pk);

#define SKEY_BYTES (4*4*2*2*6+12*4*3+6+6+5)
#define PKEY_BYTES ((65+25+25+65)*12+9)

#endif
