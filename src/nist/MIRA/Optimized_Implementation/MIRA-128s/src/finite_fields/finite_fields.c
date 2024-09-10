#include "finite_fields.h"
#include <stdio.h>
#include <stdlib.h>


gf16 gf16_add(gf16 a, gf16 b) {
	return a^b;
}

gf16 gf16_sub(gf16 a, gf16 b) {
	return a^b;
}

gf16 gf16_mul(gf16 a, gf16 b) {
	gf16 o = -(b & 1) & a;
	o ^= (-((b >> 1) & 1) & a) << 1;
	o ^= (-((b >> 2) & 1) & a) << 2;
	o ^= (-((b >> 3) & 1) & a) << 3;

	gf16 upper_part = o >> 4;

	gf16 result = upper_part ^ (upper_part << 1);

	return (o & 0xf) ^ (result & 0xf);
}

void gf16_print(gf16 e) {
	printf("%x", e);
}

//GF(16) matrices

void gf16_mat_init(gf16_mat *o, int lines, int columns) {
	*o = (gf16_mat)malloc(lines*columns*sizeof(gf16));
}

void gf16_mat_set(gf16_mat o, gf16_mat a, int lines, int columns) {
	memcpy(o, a, lines*columns*sizeof(gf16));
}

void gf16_mat_set_zero(gf16_mat o, int lines, int columns) {
	for(int l=0 ; l<lines ; l++) {
		for(int c=0 ; c<columns ; c++) {
			o[l*columns + c] = 0;
		}
	}
}

void gf16_mat_set_random(seedexpander_shake_t* ctx, gf16_mat o, int lines, int columns) {
	int bytes = (lines * columns + 1) / 2;
	uint8_t random[bytes];
	seedexpander_shake_get_bytes(ctx, random, bytes);
	for(int i=0 ; i<lines*columns/2 ; i++) {
		o[2*i] = random[i] & 0xf;
		o[2*i+1] = random[i] >> 4;
	}
	if(lines*columns % 2 == 1) o[lines*columns-1] = random[bytes-1] & 0xf;
}

void gf16_mat_add(gf16_mat o, gf16_mat a, gf16_mat b, int lines, int columns) {
	for(int i=0 ; i<lines*columns ; i++) {
		o[i] = a[i] ^ b[i];
	}
}

void gf16_mat_scalar_mul(gf16_mat o, gf16_mat a, gf16 e, int lines, int columns) {
	for(int l=0 ; l<lines ; l++) {
		for(int c=0 ; c<columns ; c++) {
			o[l*columns + c] = gf16_mul(a[l*columns + c], e);
		}
	}
}

void gf16_mat_mul(gf16_mat o, gf16_mat a, gf16_mat b, int lines, int length, int columns) {
	gf16 tmp;
	for(int l=0 ; l<lines ; l++) {
		for(int c=0 ; c<columns ; c++) {
			o[l*columns + c] = 0;
			for(int k=0 ; k<length ; k++) {
				tmp = gf16_mul(a[l*length + k], b[k*columns + c]);
				o[l*columns + c] ^= tmp;
			}
		}
	}
}

void gf16_mat_clear(gf16_mat o) {
	free(o);
}

void gf16_mat_print(gf16_mat o, int lines, int columns) {
	printf("[");
	for(int l=0 ; l<lines ; l++) {
		printf("[");
		for(int c=0 ; c<columns ; c++) {
			gf16_print(o[l*columns + c]); printf(" ");
		}
		printf("]\n");
	}
	printf("]\n");
}

void gf16_mat_to_string(uint8_t* o, const gf16_mat a, int lines, int columns) {
	int bytes = (lines * columns + 1) / 2;

	for(int i=0 ; i<lines*columns/2 ; i++) {
		o[i] = (a[2*i] & 0xf) | (a[2*i+1] << 4);
	}

	if(lines*columns % 2 == 1) o[bytes - 1] = a[lines*columns - 1] & 0xf;
}

void gf16_mat_from_string(gf16_mat o, int lines, int columns, const uint8_t *src) {
	int bytes = (lines * columns + 1) / 2;

	for(int i=0 ; i<lines*columns/2 ; i++) {
		o[2*i] = src[i] & 0xf;
		o[2*i+1] = src[i] >> 4;
	}

	if(lines*columns % 2 == 1) o[lines*columns-1] = src[bytes-1] & 0xf;
}

//GF(16)^16

void gfqm_set(gfqm o, const gfqm a) {
	for(int i=0 ; i<16 ; i++) o[i] = a[i];
}

void gfqm_set_zero(gfqm o) {
	for(int i=0 ; i<16 ; i++) o[i] = 0;
}

void gfqm_set_one(gfqm o) {
	o[0] = 1;
	for(int i=1 ; i<16 ; i++) o[i] = 0;
}

void gfqm_add(gfqm o, const gfqm a, const gfqm b) {
	for(int i=0 ; i<16 ; i++) o[i] = a[i] ^ b[i];
}

void gfqm_scalar_mul(gfqm o, const gfqm a, const gf16 e) {
	for(int i=0 ; i<16 ; i++) o[i] = gf16_mul(a[i], e);
}


void gf16to2_mul(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    uint8_t leading = gf16_mul(a[1], b[1]);
    uint8_t cnst = gf16_mul(a[0], b[0]);
    uint8_t sum_a = a[0] ^ a[1];
    uint8_t sum_b = b[0] ^ b[1];
    res[0] = cnst ^ gf16_mul(leading, 0x8);
    res[1] = gf16_mul(sum_a,sum_b) ^ cnst;
}

void gf16to2_add(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    res[0] = a[0] ^ b[0];
    res[1] = a[1] ^ b[1];
}

static void gf16to2_mul_0x80(uint8_t res[2], const uint8_t a[2]) {
    res[0] = gf16_mul(a[1], 0xC); // 0xC = 0x8*0x8
    res[1] = gf16_mul(a[0] ^ a[1], 0x8);
}

void gf16to4_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint8_t leading[2], cnst[2], sum_a[2], sum_b[2];
    gf16to2_mul(leading, &a[2], &b[2]);
    gf16to2_mul(cnst, &a[0], &b[0]);
    gf16to2_add(sum_a, &a[0], &a[2]);
    gf16to2_add(sum_b, &b[0], &b[2]);
    // Compute &res[0]
    gf16to2_mul_0x80(&res[0], leading);
    gf16to2_add(&res[0], &res[0], cnst);
    // Compute &res[2]
    gf16to2_mul(&res[2], sum_a, sum_b);
    gf16to2_add(&res[2], &res[2], cnst);
}

void gf16to4_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    res[0] = a[0] ^ b[0];
    res[1] = a[1] ^ b[1];
    res[2] = a[2] ^ b[2];
    res[3] = a[3] ^ b[3];
}

static void gf16to4_mul_0x8000(uint8_t res[4], const uint8_t a[4]) {
    uint8_t leading[2], sum_a[2];
    gf16to2_mul_0x80(leading, &a[2]);
    gf16to2_add(sum_a, &a[0], &a[2]);
    // Compute &res[0]
    gf16to2_mul_0x80(&res[0], leading);
    // Compute &res[2]
    gf16to2_mul_0x80(&res[2], sum_a);
}

void gf16to8_mul(uint8_t res[8], const uint8_t a[8], const uint8_t b[8]) {
    uint8_t leading[4], cnst[4], sum_a[4], sum_b[4];
    gf16to4_mul(leading, &a[4], &b[4]);
    gf16to4_mul(cnst, &a[0], &b[0]);
    gf16to4_add(sum_a, &a[0], &a[4]);
    gf16to4_add(sum_b, &b[0], &b[4]);
    // Compute &res[0]
    gf16to4_mul_0x8000(&res[0], leading);
    gf16to4_add(&res[0], &res[0], cnst);
    // Compute &res[4]
    gf16to4_mul(&res[4], sum_a, sum_b);
    gf16to4_add(&res[4], &res[4], cnst);
}

void gf16to8_add(uint8_t res[8], const uint8_t a[8], const uint8_t b[8]) {
    res[0] = a[0] ^ b[0];
    res[1] = a[1] ^ b[1];
    res[2] = a[2] ^ b[2];
    res[3] = a[3] ^ b[3];
    res[4] = a[4] ^ b[4];
    res[5] = a[5] ^ b[5];
    res[6] = a[6] ^ b[6];
    res[7] = a[7] ^ b[7];
}

static void gf16to8_mul_0x80000000(uint8_t res[8], const uint8_t a[8]) {
    uint8_t leading[4], sum_a[4];
    gf16to4_mul_0x8000(leading, &a[4]);
    gf16to4_add(sum_a, &a[0], &a[4]);
    // Compute &res[0]
    gf16to4_mul_0x8000(&res[0], leading);
    // Compute &res[2]
    gf16to4_mul_0x8000(&res[4], sum_a);
}

void gf16to16_mul(uint8_t res[16], const uint8_t a[16], const uint8_t b[16]) {
    uint8_t leading[8], cnst[8], sum_a[8], sum_b[8];
    gf16to8_mul(leading, &a[8], &b[8]);
    gf16to8_mul(cnst, &a[0], &b[0]);
    gf16to8_add(sum_a, &a[0], &a[8]);
    gf16to8_add(sum_b, &b[0], &b[8]);
    // Compute &res[0]
    gf16to8_mul_0x80000000(&res[0], leading);
    gf16to8_add(&res[0], &res[0], cnst);
    // Compute &res[8]
    gf16to8_mul(&res[8], sum_a, sum_b);
    gf16to8_add(&res[8], &res[8], cnst);
}

const uint8_t gf16to16_pow16_array[16*16] = {
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    9,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
    11,8,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
    7,11,9,1,1,1,0,0,0,0,0,0,0,0,0,0,
    11,11,3,8,1,1,1,0,0,0,0,0,0,0,0,0,
    12,11,15,3,9,1,1,1,0,0,0,0,0,0,0,0,
    7,6,12,3,11,8,1,1,1,0,0,0,0,0,0,0,
    2,7,7,12,7,11,9,1,1,1,0,0,0,0,0,0,
    5,6,12,9,11,11,3,8,1,1,1,0,0,0,0,0,
    0,5,8,12,12,11,15,3,9,1,1,1,0,0,0,0,
    5,10,9,15,3,14,4,3,11,8,1,1,1,0,0,0,
    10,5,8,9,10,3,15,4,7,11,9,1,1,1,0,0,
    2,0,4,12,13,14,8,9,11,11,3,8,1,1,1,0,
    2,2,14,4,4,13,12,8,12,11,15,3,9,1,1,1
};

void gf16to16_pow16(uint8_t res[16], const uint8_t a[16]) {
    for(int i=0; i<16; i++) {
        uint8_t v = 0;
        for(int j=i; j<16; j++)
            v ^= gf16_mul(a[j], gf16to16_pow16_array[16*j+i]);
        // We set res[.] only after in case than "res" and "a"
        //   point to the same memory block.
        res[i] = v;
    }
}

//Should not be called with o=a
void gfqm_pow15(gfqm o, gfqm a) {
	gfqm a2, a4, a12;

	gfqm_mul(a2, a, a); //a^2
	gfqm_mul(a4, a2, a2); //a^4
	gfqm_mul(a12, a4, a4); //a^8
	gfqm_mul(a12, a12, a4); //a^12
	gfqm_mul(o, a12, a2); //a^14
	gfqm_mul(o, o, a); //a^15
}

//Should not be called with o=a
void gfqm_pow16(gfqm o, gfqm a) {
	gf16to16_pow16(o, a);
	/*
	gfqm_mul(o, a, a); //a^2
	gfqm_mul(o, o, o); //a^4
	gfqm_mul(o, o, o); //a^8
	gfqm_mul(o, o, o); //a^16*/
}

void gfqm_mul(gfqm o, const gfqm a, const gfqm b) {
	gf16to16_mul(o, a, b);
}

void gfqm_to_string(uint8_t *o, const gfqm e) {
	for(int i=0 ; i<8 ; i++) {
		o[i] = (e[2*i] & 0xf) | (e[2*i+1] << 4);
	}
}

void gfqm_from_string(gfqm o, const uint8_t *src) {
	for(int i=0 ; i<8 ; i++) {
		o[2*i] = src[i] & 0xf;
		o[2*i+1] = src[i] >> 4;
	}
}

void gfqm_print(gfqm e) {
	printf("[");
	for (int i=0 ; i<16 ; i++) {
		gf16_print(e[i]);
		printf(" ");
	}
	printf("]");
}

void gfqm_vec_init(gfqm_vec* o, int size) {
	*o = (gfqm_vec) malloc(size*sizeof(gfqm));
}

void gfqm_vec_set(gfqm_vec o, gfqm_vec a, int size) {
	for(int i=0 ; i<size ; i++) {
		gfqm_set(o[i], a[i]);
	}
}

void gfqm_vec_set_zero(gfqm_vec o, int size) {
	for(int i=0 ; i<size ; i++) gfqm_set_zero(o[i]);
}

void gfqm_vec_add(gfqm_vec o, gfqm_vec a, gfqm_vec b, int size) {
	for(int i=0 ; i<size ; i++) {
		gfqm_add(o[i], a[i], b[i]);
	}
}

void gfqm_vec_inner_product(gfqm o, gfqm_vec a, gfqm_vec b, int size) {
	gfqm_set_zero(o);
	gfqm tmp;
	for(int i=0 ; i<size ; i++) {
		gfqm_mul(tmp, a[i], b[i]);
		gfqm_add(o, o, tmp);
	}
}

void gfqm_vec_scalar_mul(gfqm_vec o, gfqm_vec a, const gfqm e, int size) {
	for(int i=0 ; i<size ; i++) {
		gfqm_mul(o[i], a[i], e);
	}
}

void gfqm_vec_to_string(uint8_t *o, const gfqm_vec e, int size) {
	for(int i=0 ; i<16*size/2 ; i++) {
		o[i] = (e[(2*i)/16][(2*i)%16] & 0xf) | (e[(2*i+1)/16][(2*i+1)%16] << 4);
	}
}

void gfqm_vec_from_string(gfqm_vec o, int size, const uint8_t *src) {
	for(int i=0 ; i<16*size/2 ; i++) {
		o[(2*i)/16][(2*i)%16] = src[i] & 0xf;
		o[(2*i+1)/16][(2*i+1)%16] = src[i] >> 4;
	}
}

void gfqm_vec_clear(gfqm_vec o) {
	free(o);
}

//Linearized polynomials

void gfqm_qpoly_eval(gfqm o, gfqm *poly, gfqm x, int terms) {
	gfqm tmp1, tmp1_q, tmp2;

	gfqm_set(tmp1, x);
	gfqm_mul(o, poly[0], tmp1);

	for(int32_t i = 1 ; i < terms ; i++) {
		gfqm_pow16(tmp1_q, tmp1);
		gfqm_mul(tmp2, poly[i], tmp1_q);
		gfqm_add(o, o, tmp2);
		gfqm_set(tmp1, tmp1_q);
	}
}

void gfqm_qpoly_qexp(gfqm *o, gfqm *a, int terms) {
	gfqm tmp;
	gfqm_set_zero(o[0]);
	
	for(int32_t i = 0 ; i < terms ; i++) {
		gfqm_pow16(tmp, a[i]);
		gfqm_set(o[i+1], tmp);
	}
}

void gfqm_qpoly_q_minus_one_exp(gfqm *o, gfqm *a, int terms) {
	gfqm tmp;
	gfqm_set_zero(o[0]);
	
	for(int32_t i = 0 ; i < terms ; i++) {
		gfqm_pow15(tmp, a[i]);
		gfqm_set(o[i+1], tmp);
	}
}

void gfqm_qpoly_add(gfqm *o, gfqm *a, gfqm *b, int terms) {
	for(int i=0 ; i<terms ; i++) gfqm_add(o[i], a[i], b[i]);
}

void gfqm_qpoly_scalar_mul(gfqm *o, gfqm *a, gfqm e, int terms) {
	for(int i=0 ; i<terms ; i++) gfqm_mul(o[i], a[i], e);
}

void gfqm_qpoly_annihilator(gfqm *o, gfqm *support, int size) {
	gfqm eval, eval_q_minus_1;
	gfqm t[size+1];
	for(int i=0 ; i<size+1 ; i++) gfqm_set_zero(t[i]);

	//First we set P1(x) = x^q - support[0].x
	gfqm_pow15(o[0], support[0]);
	gfqm_set_one(o[1]);
	for(int i=2 ; i<size+1 ; i++) gfqm_set_zero(o[i]);

	for(int i=1 ; i<size ; i++) {
		gfqm_qpoly_qexp(t, o, i+1); //P_(i-1) ^ q
		gfqm_qpoly_eval(eval, o, support[i], i+1);
		gfqm_pow15(eval_q_minus_1, eval); //P(i-1)(support[i])^(q-1)
		gfqm_qpoly_scalar_mul(o, o, eval_q_minus_1, i+1); //P_(i-1)(support[i])^(q-1) P_(i-1)
		gfqm_qpoly_add(o, o, t, i+2); //P_(i-1) ^ q + P_(i-1)(support[i])^(q-1) P_(i-1)
	}
}

void gfqm_qpoly_print(gfqm *o, int terms) {
	for(int i=0 ; i<terms-1 ; i++) {
		gfqm_print(o[i]); printf(" X^(q^%d) + \n", i);
	}
	gfqm_print(o[terms-1]); printf(" X^(q^%d)\n", terms-1);
}