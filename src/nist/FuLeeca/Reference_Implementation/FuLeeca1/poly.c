#include <stdio.h>
#include <stdbool.h>

#include "params.h"
#include "coeff.h"
#include "poly.h"


/*************************************************
* Name:        init_zero_poly_inv
*
* Description: Set poly_inv to zero
*
* Arguments:   - poly_inv *p: Pointer to poly_inv
**************************************************/
void init_zero_poly_inv(poly_inv *p)
{
  coeff_init_zero(p->coeffs, N + N/2);
}

/*************************************************
* Name:        init_zero_poly_n
*
* Description: Set poly_n to zero
*
* Arguments:   - poly_n *p: Pointer to poly_n
**************************************************/
void init_zero_poly_n(poly_inv *p)
{
  coeff_init_zero(p->coeffs, N);
}

/*************************************************
* Name:        init_zero_poly_n_2
*
* Description: Set poly_n_2 to zero
*
* Arguments:   - poly_n_2 *p: Pointer to poly_n_2
**************************************************/
void init_zero_poly_n_2(poly_inv *p)
{
  coeff_init_zero(p->coeffs, N/2);
}

/*************************************************
* Name:        poly_mul_modp_schoolbook_2n
*
* Description: Modulo schoolbook mult. of two polys a and b of length n to poly c
*
* Arguments:   - coeff *c: Output array of of the mult
*              - const coeff *a: Pointer to input array a
*              - const coeff *b: Pointer to input array b
*              - const size_t n: Size of a and b
**************************************************/
void poly_mul_modp_schoolbook_2n(coeff *c, const coeff *a, const coeff *b, const size_t n)
{
    /* This is not optimal, think about how to improve that */
    for (size_t i=0; i<2*n; i++) {
        c[i] = 0;
    }
    coeff tmp = 0;
    for (size_t i=0; i<n; i++) {
        for (size_t j=0; j<n; j++) {
            tmp = coeff_mul_modp(a[i], b[j]);
            c[i+j] = coeff_add_modp(c[i+j], tmp);
        }
    }
}


/*************************************************
* Name:        poly_2n_red
*
* Description: Reduce poly of lenght 2n to length n
*
* Arguments:   - coeff *c: Output array length n
*              - const coeff *a: Input array length 2*n
*              - const size_t n: Size of c
**************************************************/
void poly_2n_red(coeff *c, const coeff *a, const size_t n)
{
  for (size_t i=n; i<2*n; i++)
  {
        c[i-n] = coeff_add_modp(a[i-n], a[i]);
  }
}


/*************************************************
* Name:        poly_mul_modp_karatsuba
*
* Description: Karatsuba multiplication of a and b to c with length n and recursion threshold th
*
* Arguments:   - coeff *c: Output array length n
*              - const coeff *a: Input array length n
*              - const coeff *b: Input array length n
*              - const size_t n: Size of c
*              - const size_t th: Recursion threshold
**************************************************/
void poly_mul_modp_karatsuba(coeff *c, const coeff *a, const coeff *b, const size_t n, const size_t th)
{
    /* Size dimensions - require floored and ceiled halves for odd n */
    size_t nh = n>>1;   // lower half (floored)
    size_t nu = n-nh;   // upper half (ceiled)
    size_t dnh = nh<<1; // doubled nh
    size_t i;
    coeff buf[2*nu];

    /* If threshold is reached, switch to schoolbook multiplication */
    if (n <= th) {
        poly_mul_modp_schoolbook_2n(c, a, b, n);
        return;
    }

    /* Split poly into upper half and lower half */
    const coeff *a0 = &a[0];
    const coeff *a1 = &a[nh];
    const coeff *b0 = &b[0];
    const coeff *b1 = &b[nh];

    coeff *a1a0 = &c[0];
    coeff *b1b0 = &c[nu];

    /* In case of odd splitting, there's no a0/b0 coefficient at position nu-1 */
    a1a0[nu-1] = a1[nu-1];
    b1b0[nu-1] = b1[nu-1];
    for (i=0; i<nh; i++) {
        a1a0[i] = coeff_add_modp(a1[i], a0[i]);
        b1b0[i] = coeff_add_modp(b1[i], b0[i]);
    }

    /* Recursively compute a1a0 * b1b0, a0b0, a1b1 */
    poly_mul_modp_karatsuba(buf, a1a0, b1b0, nu, th);
    poly_mul_modp_karatsuba(&c[0], a0, b0, nh, th);
    poly_mul_modp_karatsuba(&c[dnh], a1, b1, nu, th);

    /* As mentioned above, only dnh - 1 coefficients are computed, */
    /* thus position dnh - 1 must be zero padded */
    c[dnh-1] = 0;

    /* (a1a0*b1b0) - a1b1 - a0b0*/
    for (i=0; i<2*nu-1; i++) {
        buf[i] = coeff_sub_modp(buf[i], c[dnh+i]);
    }
    for (i=0; i<dnh-1; i++) {
        buf[i] = coeff_sub_modp(buf[i], c[i]);
    }
    /* Add everything */
    for (i=0; i<2*nu-1; i++) {
        c[nh+i] = coeff_add_modp(buf[i], c[nh+i]);
    }
}


/*************************************************
* Name:        poly_mul_modp
*
* Description: Multiplication of two poly_n_2 to poly_n_2
*
* Arguments:   - poly_n_2 *c: Output poly
*              - poly_n_2 *a: Input multiplicand
*              - poly_n_2 *b: Input multiplicand
**************************************************/
void poly_mul_modp(poly_n_2 *c, const poly_n_2 *a, const poly_n_2 *b) 
{
    /* Buffer of dimension N */
    coeff tmp[N];

    /* Karatsuba multiplication returning N sized polynomial */
    poly_mul_modp_karatsuba(tmp, a->coeffs, b->coeffs, N/2, 16);

    /* Reduce polynomial with cyclic shift to N/2 again */
    poly_2n_red(c->coeffs, tmp, N/2);
}

/*************************************************
* Name:        poly_mul_modp_n
*
* Description: Multiplication of two poly_n to poly_n
*
* Arguments:   - poly_n *c: Output poly
*              - poly_n *a: Input multiplicand
*              - poly_n *b: Input multiplicand
**************************************************/
void poly_mul_modp_n(poly_n *c, const poly_n *a, const poly_n *b) 
{
    /* Buffer of dimension N */
    coeff tmp[2*N];

    /* Karatsuba multiplication returning N sized polynomial */
    poly_mul_modp_karatsuba(tmp, a->coeffs, b->coeffs, N, 16);

    /* Reduce polynomial with cyclic shift to N/2 again */
    poly_2n_red(c->coeffs, tmp, N);
}

/*************************************************
* Name:        poly_n_to_poly_s_d_n
*
* Description: Convert unsigned poly_n to signed poly_s_d_n with double coeffs
*
* Arguments:   - poly_s_d_n *out: Output poly
*              - poly_n *in: Input Poly
**************************************************/
void poly_n_to_poly_s_d_n(poly_s_d_n *out, const poly_n *in)
{
  size_t i;
  s_d_coeff sign = 0;
  for(i = 0; i < N; i++)
  {
    sign = (in->coeffs[i] > (P-1)/2);
    out->coeffs[i] = ((((s_d_coeff)(in->coeffs[i]) - P) & (-sign)) | ((s_d_coeff)(in->coeffs[i]) & (~(-sign))));
  }
}

/*************************************************
* Name:        poly_n_2_to_poly_s_n_2
*
* Description: Convert unsigned poly_n_2 to signed poly_s_n_2
*
* Arguments:   - poly_s_n_2 *out: Output poly
*              - poly_n_2 *in: Input Poly
**************************************************/
void poly_n_2_to_poly_s_n_2(poly_s_n_2 *out, const poly_n_2 *in)
{
  size_t i;
  s_coeff sign = 0;
  for(i = 0; i < N/2; i++)
  {
    sign = (in->coeffs[i] > (P-1)/2);
    out->coeffs[i] = ((((s_coeff)(in->coeffs[i]) - P) & (-sign)) | ((s_coeff)(in->coeffs[i]) & (~(-sign))));
  }
}

/*************************************************
* Name:        poly_to_signed
*
* Description: Convert unsigned poly_n to signed coeff array
*
* Arguments:   - c_coeff *out: Output array
*              - poly_n *in: Input Poly
*              - const size_t inlen: length of input
**************************************************/
void poly_to_signed(s_coeff *out, const poly_n *in, const size_t inlen) {
    for (size_t i=0; i<inlen; i++) {
        if (in->coeffs[i] > (P-1)/2) {
            out[i] = in->coeffs[i] - P;
        } else {
            out[i] = in->coeffs[i];
        }
    }
}


/*************************************************
* Name:        signed_to_poly
*
* Description: Convert unsigned poly_n to signed coeff array
*
* Arguments:   - poly_n *out: Output Poly
*              - c_coeff *in: Input array
*              - const size_t inlen: length of input
**************************************************/
void signed_to_poly(poly_n_2 *out, const s_coeff *in, const size_t inlen) {
    for (size_t i=0; i<inlen; i++) {
        out->coeffs[i] = (coeff) in[i];
        if (in[i] < 0) {
            out->coeffs[i] = (coeff) in[i] + P;
        }
    }
}

/*************************************************
* Name:        print_poly_n_2
*
* Description: Print polynomial of length N/2
*
* Arguments:   - const poly_n_2 *p: polynomial to be printed
**************************************************/
void print_poly_n_2(const poly_n_2 *p)
{
  print_coeff(&(p->coeffs[0]), N/2);
}

/*************************************************
* Name:        print_poly_s_d_n
*
* Description: Print polynomial of length N
*
* Arguments:   - const poly_s_d_n *p: polynomial to be printed
**************************************************/
void print_poly_s_d_n(const poly_s_d_n *p)
{
  print_s_d_coeff(&(p->coeffs[0]), N);
}

/*************************************************
* Name:        print_poly_n
*
* Description: Print polynomial of length N
*
* Arguments:   - const poly_n *p: polynomial to be printed
**************************************************/
void print_poly_n(const poly_n *p)
{
  print_coeff(&(p->coeffs[0]), N);
}

/*************************************************
* Name:        print_poly_inv
*
* Description: Print polynomial of length N + N/2
*
* Arguments:   - const poly_inv *p: polynomial to be printed
**************************************************/
void print_poly_inv(const poly_inv *p)
{
  print_coeff(&(p->coeffs[0]), N + N/2);
}
