#include "../sign.h"
#include "../poly.h"
#include "../fips202.h"
#include "../utils.h"
#include "../params.h"
#include "../randombytes.h"

#include <string.h>
#include <stdio.h>

#define NTESTS 1000

int main()
{
	size_t j;
	poly_n_2 a_sum = {0}, b_sum = {0}, a_inv_b_sum = {0};
	for(j = 0; j < NTESTS; j++)
	{
		uint8_t pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
		crypto_sign_keypair(pk, sk);
	
		//Unpack secret key
	        poly_n_2 a, b, a_inv_b;
	        memcpy(&(a.coeffs), sk, sizeof(a.coeffs));
	        memcpy(&(b.coeffs), sk + sizeof(a.coeffs), sizeof(b.coeffs));
		memcpy(&(a_inv_b.coeffs), pk, sizeof(a_inv_b.coeffs));
		print_poly_n_2(&a);
		print_poly_n_2(&b);
		print_poly_n_2(&a_inv_b);
		
		size_t weight_a = 0, weight_b = 0;
		for(int i = 0; i < N/2; i++) {
			weight_a += coeff_lee_weight(a.coeffs[i]);
			weight_b += coeff_lee_weight(b.coeffs[i]);
			a_sum.coeffs[i] = coeff_add_modp(a_sum.coeffs[i], a.coeffs[i]);
			b_sum.coeffs[i] = coeff_add_modp(b_sum.coeffs[i], b.coeffs[i]);
			a_inv_b_sum.coeffs[i] = coeff_add_modp(a_inv_b_sum.coeffs[i], a_inv_b.coeffs[i]);
		}
		printf("Lee weight of a: %lu, b: %lu\n", weight_a, weight_b);
	}
	for(size_t i = 0; i < N/2; i++) {
		printf("Sum of lee weights of a: %u, b: %u, a_inv_b: %u\n", a_sum.coeffs[i], b_sum.coeffs[i], a_inv_b_sum.coeffs[i]);
	}
}
