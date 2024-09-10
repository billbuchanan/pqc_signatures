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

//GF(16)^23

void gfqm_set(gfqm o, const gfqm a) {
	for(int i=0 ; i<23 ; i++) o[i] = a[i];
}

void gfqm_set_zero(gfqm o) {
	for(int i=0 ; i<23 ; i++) o[i] = 0;
}

void gfqm_set_one(gfqm o) {
	o[0] = 1;
	for(int i=1 ; i<23 ; i++) o[i] = 0;
}

void gfqm_add(gfqm o, const gfqm a, const gfqm b) {
	for(int i=0 ; i<23 ; i++) o[i] = a[i] ^ b[i];
}

void gfqm_scalar_mul(gfqm o, const gfqm a, const gf16 e) {
	for(int i=0 ; i<23 ; i++) o[i] = gf16_mul(a[i], e);
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
	gfqm_mul(o, a, a); //a^2
	gfqm_mul(o, o, o); //a^4
	gfqm_mul(o, o, o); //a^8
	gfqm_mul(o, o, o); //a^16
}

void gfqm_mul(gfqm o, const gfqm a, const gfqm b) {
	gf16 product[45];
	gfqm tmp;
	for(int i=1 ; i<45 ; i++) product[i] = 0;
	gfqm_scalar_mul(product, a, b[0]);

	//Mul
	for(int i=1 ; i<23 ; i++) {
		gfqm_scalar_mul(tmp, a, b[i]);
		gfqm_add(product + i, product + i, tmp);
	}

	//Red
	for(int i=44 ; i>=23 ; i--) {
		product[i - 23] ^= product[i];
		product[i - 18] ^= product[i];
		//product[i] = 0;
	}

	for(int i=0 ; i<23 ; i++) o[i] = product[i];
}

void gfqm_to_string(uint8_t *o, const gfqm e) {
	for(int i=0 ; i<11 ; i++) {
		o[i] = (e[2*i] & 0xf) | (e[2*i+1] << 4);
	}

	o[11] = e[22] & 0xf;
}

void gfqm_from_string(gfqm o, const uint8_t *src) {
	for(int i=0 ; i<11 ; i++) {
		o[2*i] = src[i] & 0xf;
		o[2*i+1] = src[i] >> 4;
	}

	o[22] = src[11] & 0xf;
}

void gfqm_print(gfqm e) {
	printf("[");
	for (int i=0 ; i<23 ; i++) {
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
	int bytes = (23 * size + 1) / 2;

	for(int i=0 ; i<23*size/2 ; i++) {
		o[i] = (e[(2*i)/23][(2*i)%23] & 0xf) | (e[(2*i+1)/23][(2*i+1)%23] << 4);
	}

	if(23 * size % 2 == 1) o[bytes - 1] = e[size-1][22] & 0xf;
}

void gfqm_vec_from_string(gfqm_vec o, int size, const uint8_t *src) {
	int bytes = (23 * size + 1) / 2;

	for(int i=0 ; i<23*size/2 ; i++) {
		o[(2*i)/23][(2*i)%23] = src[i] & 0xf;
		o[(2*i+1)/23][(2*i+1)%23] = src[i] >> 4;
	}

	if(23 * size % 2 == 1) o[size-1][22] = src[bytes-1] & 0xf;
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