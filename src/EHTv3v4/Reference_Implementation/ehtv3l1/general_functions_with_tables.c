#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "tables.h"

/**
 * This function finds the modular multiplicative inverse of a given number in the range [0, Q) under modulus Q.
 * The modular multiplicative inverse of 'a' modulo Q is an integer 'b' such that (a*b) mod Q = 1.
 * This function uses a precalculated table 'inverse_mod_q' of modular inverses for all possible 
 * values under Q, and returns the corresponding value for the input 'a'.
 *
 * @param a The number to find the inverse of.
 * @return The modular multiplicative inverse of 'a'.
 */
unsigned char inverse(int a)
{
    return inverse_mod_q[a];
}

/**
 * This function computes the product of two numbers in the range [0, Q) under modulus Q.
 * The operation performed is (a * b) mod Q.
 * This function uses a precalculated 2D table 'product_mod_q' of products for all possible pairs
 * of values under Q, and returns the value corresponding to the pair (a, b).
 *
 * @param a The first number.
 * @param b The second number.
 * @return The product of 'a' and 'b' under modulus Q.
 */
unsigned char prod(int a, int b)
{
    return product_mod_q[a][b];
}

/**
 * This function computes the sum of two numbers in the range [0, Q) under modulus Q.
 * The operation performed is (a + b) mod Q.
 * This function uses a precalculated 2D table 'sum_mod_q' of sums for all possible pairs
 * of values under Q, and returns the value corresponding to the pair (a, b).
 *
 * @param a The first number.
 * @param b The second number.
 * @return The sum of 'a' and 'b' under modulus Q.
 */
unsigned char add(int a, int b)
{
    return sum_mod_q[a][b];
}

/**
 * This function computes the difference of two numbers under modulus Q.
 * The operation performed is (a - b) mod Q.
 * This function uses a precalculated 2D table 'dif_mod_q' of difference for all possible pairs
 * of values under Q, and returns the value corresponding to the pair (a, b).
 *
 * @param a The first number.
 * @param b The second number.
 * @return The difference of 'a' and 'b' under modulus Q.
 */
unsigned char sub(int a, int b)
{
    return dif_mod_q[a][b];
}


