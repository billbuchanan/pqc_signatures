#ifndef POLY_H
#define POLY_H

#include "params.h"
#include "coeff.h"

typedef struct {
	coeff coeffs[N/2];
} poly_n_2;

typedef struct {
	s_coeff coeffs[N/2];
} poly_s_n_2;

typedef struct {
	coeff coeffs[N];
} poly_n;

typedef struct {
	s_coeff coeffs[N];
} poly_s_n;

typedef struct {
	s_d_coeff coeffs[N];
} poly_s_d_n;

typedef struct {
	coeff coeffs[N + N/2];
} poly_inv;

#define init_zero_poly_inv FULEECA_NAMESPACE(_init_zero_poly_inv)
void init_zero_poly_inv(poly_inv *p);

#define init_zero_poly_n FULEECA_NAMESPACE(_init_zero_poly_n)
void init_zero_poly_n(poly_inv *p);

#define init_zero_poly_n_2 FULEECA_NAMESPACE(_init_zero_poly_n_2)
void init_zero_poly_n_2(poly_inv *p);

#define poly_mul_modp_schoolbook_2n FULEECA_NAMESPACE(_poly_mul_modp_schoolbook_2n)
void poly_mul_modp_schoolbook_2n(coeff *c, const coeff *a, const coeff *b, const size_t n);

#define poly_mul_modp_karatsuba FULEECA_NAMESPACE(_poly_mul_modp_karatsuba)
void poly_mul_modp_karatsuba(coeff *c, const coeff *a, const coeff *b, const size_t n, const size_t th);

#define poly_2n_red FULEECA_NAMESPACE(_poly_2n_red)
void poly_2n_red(coeff *c, const coeff *a, const size_t n);

#define poly_mul_modp FULEECA_NAMESPACE(_poly_mul_modp)
void poly_mul_modp(poly_n_2 *c, const poly_n_2 *a, const poly_n_2 *b);

#define poly_mul_modp_n FULEECA_NAMESPACE(_poly_mul_modp_n)
void poly_mul_modp_n(poly_n *c, const poly_n *a, const poly_n *b);

#define poly_n_to_poly_s_d_n FULEECA_NAMESPACE(_poly_n_to_poly_s_d_n)
void poly_n_to_poly_s_d_n(poly_s_d_n *out, const poly_n *in);

#define poly_n_2_to_poly_s_n_2 FULEECA_NAMESPACE(_poly_n_2_to_poly_s_n_2)
void poly_n_2_to_poly_s_n_2(poly_s_n_2 *out, const poly_n_2 *in);

#define poly_to_signed FULEECA_NAMESPACE(_poly_to_signed)
void poly_to_signed(s_coeff *out, const poly_n *in, const size_t inlen);

#define signed_to_poly FULEECA_NAMESPACE(_signed_to_poly)
void signed_to_poly(poly_n_2 *out, const s_coeff *in, const size_t inlen);


#define print_poly_n_2 FULEECA_NAMESPACE(_print_poly_n_2)
void print_poly_n_2(const poly_n_2 *p);

#define print_poly_n FULEECA_NAMESPACE(_print_poly_n)
void print_poly_n(const poly_n *p);

#define print_poly_s_d_n FULEECA_NAMESPACE(_print_poly_s_d_n)
void print_poly_s_d_n(const poly_s_d_n *p);

#define print_poly_inv FULEECA_NAMESPACE(_print_poly_inv)
void print_poly_inv(const poly_inv *p);

#endif
