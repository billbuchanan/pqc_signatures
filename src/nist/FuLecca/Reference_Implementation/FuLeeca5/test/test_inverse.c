#include "../poly.h"
#include "../coeff.h"
#include "../utils.h"
#include "../fips202.h"
#include "../randombytes.h"
#include "fix_value_init.h"

#include <stdio.h>

#define NTESTS 100000

int main()
{
	poly_n_2 a = {0}, a_inv = {0}, verif_inverse = {0};
	
	uint8_t seed[SEEDBYTES];

	int successfull_inverse = 0;

	for(int i = 0; i < NTESTS; i++)
	{
		for(int j = 0; j < N/2; j++)
		{
			a.coeffs[j] = 0;
			a_inv.coeffs[j] = 0;
			verif_inverse.coeffs[j] = 0;
		}

		randombytes(seed, sizeof(seed));
		keccak_state state;

		shake128_init(&state);
		shake128_absorb(&state, seed, SEEDBYTES);
		shake128_finalize(&state);

		poly_sample_from_typical_set(&a, &state);

		extended_euclidean_algorithm(&a_inv, &a);

		poly_mul_modp(&verif_inverse, &a_inv, &a);
	
		int inverse_exists = verif_inverse.coeffs[0];
	        for(int j = 1; j < N/2; j++)
	        {
	                if(verif_inverse.coeffs[j] != 0)
	                {
	                        inverse_exists = 0;
	                }
	        }
	successfull_inverse += inverse_exists;
	}
	printf("Number of successfull inverses in %d tests: %d\n", NTESTS, successfull_inverse);
}
