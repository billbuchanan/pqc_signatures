#include <stdio.h>
#include <stdlib.h>

#include "coeff.h"

/*************************************************
* Name:        print_coeff
*
* Description: Print coeff array of length n
*
* Arguments:   - const coeff *p: Pointer to array
*              - const size_t n: length of array
**************************************************/
void print_coeff(const coeff *p, const size_t n)
{
    printf("%d", p[0]);
    for (size_t i=1; i<n-1; i++) {
        printf(",%d", p[i]);
    }
    printf(",%d\n", p[n-1]);
}

/*************************************************
* Name:        print_s_coeff
*
* Description: Print s_coeff array of length n
*
* Arguments:   - const s_coeff *p: Pointer to array
*              - const size_t n: length of array
**************************************************/
void print_s_coeff(const s_coeff *p, const size_t n)
{
    printf("%i", p[0]);
    for (size_t i=1; i<n-1; i++) {
        printf(",%i", p[i]);
    }
    printf(",%i\n", p[n-1]);
}

/*************************************************
* Name:        print_s_d_coeff
*
* Description: Print s_d_coeff array of length n
*
* Arguments:   - const s_d_coeff *p: Pointer to array
*              - const size_t n: length of array
**************************************************/
void print_s_d_coeff(const s_d_coeff *p, const size_t n)
{
    printf("%i", p[0]);
    for (size_t i=1; i<n-1; i++) {
        printf(",%i", p[i]);
    }
    printf(",%i\n", p[n-1]);
}

/*************************************************
* Name:        coeff_init_zero
*
* Description: Initializes array of coeffs of length n with zero
*
* Arguments:   - coeff *p: Pointer to array
*              - const size_t n: length of array
**************************************************/
void coeff_init_zero(coeff *p, const size_t n)
{
	for(size_t i = 0; i < n; i++)
	{
		*(p + i) = 0;
	}
}


/*************************************************
* Name:        coeff_red_modp
*
* Description: Barrett Reduction of d_coeff
*
* Arguments:   - const d_coeff a: Coeff to be reduced
*              - return coeff: reduced coeff
**************************************************/
coeff coeff_red_modp(const d_coeff a)
{
    q_coeff tmp = (q_coeff) a;

    /* a = a - (a*m >> k)*P */
    tmp -= ((tmp * BARRETT_M) >> BARRETT_K) * P;

    /* Barrett output is [0, 2P), so do conditional subtraction */
    tmp -= P;
    tmp += P & -(tmp >> (Q_COEFF_SIZE-1));
    
    return (coeff) tmp;
}

/*************************************************
* Name:        coeff_add_modp
*
* Description: Modulo add of two coeffs
*
* Arguments:   - const coeff a: Coeff a to be added
*              - const coeff b: Coeff b to be added
*              - return coeff: added coeffs
**************************************************/
coeff coeff_add_modp(const coeff a, const coeff b) 
{
    d_coeff tmp;

    tmp = a + b - P;
    tmp += P & -(tmp >> (D_COEFF_SIZE-1));
    return (coeff) tmp;
}

/*************************************************
* Name:        coeff_sub_modp
*
* Description: Modulo subtraction of two coeffs
*
* Arguments:   - const coeff a: Coeff a to be subtracted from
*              - const coeff b: Coeff b to be subtracted
*              - return coeff: subtracted coeffs
**************************************************/
coeff coeff_sub_modp(const coeff a, const coeff b) 
{
    d_coeff tmp;

    tmp = a - b;
    tmp += P & -(tmp >> (D_COEFF_SIZE-1));
    return (coeff) tmp;
}


/*************************************************
* Name:        coeff_mul_modp
*
* Description: Modulo multiplication of two coeffs
*
* Arguments:   - const coeff a: Coeff a to be multiplied
*              - const coeff b: Coeff b to be multiplied
*              - return coeff: multiplied coeffs
**************************************************/
coeff coeff_mul_modp(const coeff a, const coeff b)
{
    d_coeff tmp = (d_coeff) a * b;
    return coeff_red_modp(tmp);
}

/*************************************************
* Name:        coeff_lee_weight
*
* Description: Calc. lee weight of coeff
*
* Arguments:   - const coeff a: Input coeff
*              - return coeff: lee weight of a
**************************************************/
coeff coeff_lee_weight(const coeff a) {
	coeff a_neg = P - a;
	return (a & -(a_neg > a)) | (a_neg & ~(-(a_neg > a)));
}

/*************************************************
* Name:        s_coeff_lee_weight
*
* Description: Calc. lee weight of s_coeff
*
* Arguments:   - const s_coeff a: Input coeff
*              - return coeff: lee weight of a
**************************************************/
coeff s_coeff_lee_weight(const s_coeff a) {
	return (coeff)(((a + P) & -(a < 0)) | (a & ~(-(a < 0))));
}

/*************************************************
* Name:        s_d_coeff_lee_weight
*
* Description: Calc. lee weight of s_d_coeff
*
* Arguments:   - const s_d_coeff a: Input coeff
*              - return d_coeff: lee weight of a
**************************************************/
d_coeff s_d_coeff_lee_weight(const s_d_coeff a) {
	return (d_coeff)abs(a);
}
