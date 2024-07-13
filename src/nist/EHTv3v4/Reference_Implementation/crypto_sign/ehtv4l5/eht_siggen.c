#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "common_functions.h"

/**
 * This function stores the message (m) and signature (x) to the signed message (sm).
 * The length of the signed message (smlen) is updated to that of the message length + the signature length
 *
 * @param m A pointer to the message.
 * @param mlen The length of the message.
 * @param x A 3D pointer (group ring matrix) to the signature x.
 * @param sm A pointer to the signed message.
 * @param smlen A pointer to the length of the signed message.
 */
void mx_to_sm(const unsigned char* m, unsigned long long mlen, unsigned short*** x, unsigned char* sm, unsigned long long* smlen)
{
	// Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = N*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Zero-out the sm vector
	for(int i=0; i<size_char; i++)
	{
		sm[i] = 0;
	}
	
	// Convert each element of x from base Q to base 256
	for(int i=0; i<N; i++)
	{
		for(int k=0; k<R; k++)
		{
			int carry = x[i][0][k];
			
			for(int l=size_char-1; l>=0; l--)
			{
				int temp = sm[l]*base_Q + carry;
	            sm[l] = temp%base_char;
	            carry = temp/base_char;
			}
		}
	}
	
	// Update the length of the signed message
	*smlen = mlen + size_char;
	
	// Append the message in the signed message
	for(int i=0; i<mlen; i++)
	{
		sm[size_char+i] = m[i];
	}
}

/**
 * This function converts part of the secret key to the base of Q and stores it in inversions which contains information about the inverses of the diagonal elements of C1.
 *
 * @param sk A pointer to the secret key.
 * @param inversions A pointer to the resulting vector.
 */
void sk_to_inversions(const unsigned char* sk, unsigned short* inversions)
{
	// Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = 2+2*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Zero-out the inversions vector
	zero_vector(size_Q, inversions);
	
	// Loop through part of the secret key, treating it as a number in base 256
	for(int l=48; l<48+size_char; l++)
    {
        int carry = sk[l];
        
        for(int i=size_Q-1; i>=0; i--)
        {
            int temp = inversions[i]*base_char + carry;
            inversions[i] = temp%base_Q;
            carry = temp/base_Q;
        }
    }
}

/**
 * The function solves for C1invh (C1inv*h) and C1invC2 (C1inv*C2) from the given C1 matrix and inversions stored in the secret key (sk).
 * It uses Gauss-Jordan elimination to bring C1 to its reduced row echelon form while applying the same operations on h and C2 to get C1inv*h and C1inv*C2
 * 
 * @param C1 A 3D pointer to the matrix C1.
 * @param C1invh A 3D pointer to the matrix C1inv*h.
 * @param C1invC2 A 3D pointer to the matrix C1inv*C2.
 * @param sk A pointer to the secret key.
 */
void solve_C1invh_and_C1invC2(unsigned short*** C1, unsigned short*** C1invh, unsigned short*** C1invC2, const unsigned char* sk)
{
	// Temporal arrays for intermediate computations.
	unsigned short inversions[2+2*R];
	unsigned short inverse[R];
	unsigned short product[R];
	unsigned short pivot[R];
	
	// Retrieve stored inversions in sk
	sk_to_inversions(sk, inversions);
	
	// Loop over the matrix C1's rows.
	for(int i=0; i<M; i++)
	{	
		for(int s=i; s<M; s++)
		{
			if(is_gr_empty(C1[s][i])==0)
			{
				if(s!=i)
				{
					for(int j=i; j<M; j++)
					{
						for(int k=0; k<R; k++)
						{
							int t = C1[i][j][k];
							C1[i][j][k] = C1[s][j][k];
							C1[s][j][k] = t;
						}
					}
					
					for(int k=0; k<R; k++)
					{
						int t = C1invC2[i][0][k];
						C1invC2[i][0][k] = C1invC2[s][0][k];
						C1invC2[s][0][k] = t;
					}
					
					for(int k=0; k<R; k++)
					{
						int t = C1invh[i][0][k];
						C1invh[i][0][k] = C1invh[s][0][k];
						C1invh[s][0][k] = t;
					}
				}
				
				break;
			}
		}
		
		// Get the current diagonal elements inverse from the inversions vector.
		if(i==0)
		{
			zero_vector(R, inverse);
			inverse[inversions[0]] = inversions[1];
		}
		else
		{
			for(int k=0; k<R; k++)
			{
				inverse[k] = inversions[2+R*(i-1)+k];
			}
		}
		
		// Multipling the inverse to row i elements of C1
		C1[i][i][0] = 1;
		for(int k=1; k<R; k++)
		{
			C1[i][i][k] = 0;
		}
		
		for(int j=i+1; j<M; j++)
		{
			product_in_Gq(inverse, C1[i][j], product, 1);
				
			for(int k=0; k<R; k++)
			{
				C1[i][j][k] = product[k];
			}
		}
		
		product_in_Gq(inverse, C1invC2[i][0], product, 1);
				
		for(int k=0; k<R; k++)
		{
			C1invC2[i][0][k] = product[k];
		}
		
		product_in_Gq(inverse, C1invh[i][0], product, 1);
			
		for(int k=0; k<R; k++)
		{
			C1invh[i][0][k] = product[k];
		}

		// Get 0's below and above current diagonal element of C1
		for(int j=0; j<M; j++)
		{
			if(j!=i)
			{
				for(int k=0; k<R; k++)
				{
					pivot[k] = C1[j][i][k];
				}
				
				for(int s=0; s<M; s++)
				{
					product_in_Gq(pivot, C1[i][s], product, 1);
					
					for(int k=0; k<R; k++)
					{
						C1[j][s][k] = s_mod_q(C1[j][s][k] - product[k]);
					}
				}
				
				product_in_Gq(pivot, C1invC2[i][0], product, 1);
					
				for(int k=0; k<R; k++)
				{
					C1invC2[j][0][k] = s_mod_q(C1invC2[j][0][k] - product[k]);
				}
				
				product_in_Gq(pivot, C1invh[i][0], product, 1);
				
				for(int k=0; k<R; k++)
				{
					C1invh[j][0][k] = s_mod_q(C1invh[j][0][k] - product[k]);
				}
			}
		}
	}
}

/**
 * The function randomizes tail (K*N-M) entries of `a` (`a2` - the last element which is a group ring of size r) and solves for the top (m) portion of `a` (`a1`).
 * It uses C1invh (C1inv*h) and C1invC2 (C1inv*C2) that were computed in the previous step to speed up solving for `a1` in the following equation:  C1*a1 + C2*a2 = h --->  a1 = C1inv*h - C1inv*C2*a2
 *
 * @param C1invh A 3D pointer to the matrix C1inv*h.
 * @param C1invC2 A 3D pointer to the matrix C1inv*C2
 * @param a A 3D pointer to `a`.
 */
void solve_a(unsigned short*** C1invh, unsigned short*** C1invC2, unsigned short*** a)
{
	// To store C1inv*C2*a2 at index i
	unsigned short product[R]; 
	
	// The tail entries of `a` (`a2`) changes if condition max_l(e) <= s is not met
	for(int k=0; k<R; k++)
	{
		a[M][0][k] = NIST_rng(Q);
	}
	
	// Compute a1
	for(int i=0; i<M; i++)
	{
		product_in_Gq(C1invC2[i][0], a[M][0], product, 1);
		
		for(int k=0; k<R; k++)
		{
			a[i][0][k] = s_mod_q(C1invh[i][0][k] - product[k]);
		}
	}
}

/**
 * This function solves for the group ring matrix `z` (containing values 'u') based on the algorithm described in the description at 1.3.2. and 7.4.
 *
 * @param T A 3D pointer to matrix `T`.
 * @param a A 3D pointer to matrix `a`.
 * @param y A 3D pointer to vector `y`.
 * @param z A 3D pointer to the vector `z` where the results will be stored.
 */
void solve_z(unsigned short*** T, unsigned short*** a, unsigned short*** y, unsigned short*** z)
{
	unsigned short prodsum1[R];
	unsigned short prodsum2[R];
	
	// modification required here if changing security parameter K
	for(int i=0; i<N; i++)
	{
		zero_vector(R, prodsum1);
		zero_vector(R, prodsum2);
		
		for(int j=0; j<i; j++)
		{
			product_in_Gq(T[K*i+0][j], y[j][0], prodsum1, 0);
			product_in_Gq(T[K*i+1][j], y[j][0], prodsum2, 0);
		}
		
		for(int k=0; k<R; k++)
		{
			int ax = mod_q(a[K*i+1][0][k] - prodsum2[k] - TUPPLE[1]*(a[K*i][0][k] - prodsum1[k]));
			ax = (ax<=Q/2)?ax:ax-Q;
			
			int y1 = ax/TUPPLE[1];
			int y2 = ax%TUPPLE[1];
			
			if(abs(y2)<=C)
			{
				z[K*i+1][0][k] = mod_q(y2);
				z[K*i][0][k] = mod_q(-1*y1);
			}
			else
			{
				z[K*i+1][0][k] = mod_q(y2 - signum(ax)*TUPPLE[1]);
				z[K*i][0][k] = mod_q(-1*(y1 + signum(ax)));
			}
			
			y[i][0][k] = mod_q(a[K*i][0][k] - prodsum1[k] - z[K*i][0][k]); // u
		}	
	}
}

/**
 * Function compute_x calculates x (the signature) by performing sequential operations with the components of group ring matrix B and group ring matrix y, as such:
 * x = B_u*B_x*B_y*B_z*y (from right to left)
 *
 * @param B_u Pointer to an array representing matrix B_u.
 * @param B_x Pointer to an array representing matrix B_x.
 * @param B_y Pointer to an array representing matrix B_y.
 * @param B_z Pointer to an array representing matrix B_z.
 * @param y A pointer to a 3D array representing the matrix y.
 * @param x A pointer to a 3D array representing the matrix x.
 */
void compute_x(unsigned short* B_u, unsigned short* B_x, unsigned short* B_y, unsigned short* B_z, unsigned short*** y, unsigned short*** x)
{
	// Temporal variables for intermediate computations
	unsigned short t0[R];
	unsigned short t1[R];
	
	// B_z*y
	// Compute the product of B_z and y, stored in t0.
    // product_in_Gq is a helper function that performs multiplication in Gq.
    // Then the elements of t0 and y[1][0] are summed modulo Q and stored in t0.
    // Also, the values from y[0][0] are stored in t1 for the next steps.
	product_in_Gq(B_z, y[0][0], t0, 1);
	for(int k=0; k<R; k++)
	{
		t0[k] = a_mod_q(t0[k]+y[1][0][k]);
		t1[k] = y[0][0][k];
	}
	
	// B_y*B_z*y
	// Compute the product of B_y and the previously obtained t0, stored in x[1][0].
    // Then the elements of t0 and t1 are stored in x[0][0] and x[1][0] respectively.
	product_in_Gq(B_y, t0, x[1][0], 1);
	for(int k=0; k<R; k++)
	{
		x[0][0][k] = t0[k];
		x[1][0][k] = a_mod_q(x[1][0][k] + t1[k]);
	}
	
	// B_x*B_y*B_z*y
	// Continue the chain by calculating the product of B_x and x[1][0], stored in t0.
    // Then the elements of t0 and x[0][0] are summed modulo Q and stored in t0,
    // while the values from x[1][0] are stored in t1 for the next steps.
	product_in_Gq(B_x, x[1][0], t0, 1);
	for(int k=0; k<R; k++)
	{
		t0[k] = a_mod_q(t0[k] + x[0][0][k]);
		t1[k] = x[1][0][k];
	}
	
	// B_u*B_x*B_y*B_z*y
	// Finally, compute the product of B_u and the previously obtained t0, stored in x[0][0].
    // Then the elements of x[0][0] and t1, and t0 are stored in x[0][0] and x[1][0] respectively.
	product_in_Gq(B_u, t0, x[0][0], 1);
	for(int k=0; k<R; k++)
	{
		x[0][0][k] = a_mod_q(x[0][0][k] + t1[k]);
		x[1][0][k] = t0[k];
	}
}


/**
 * This function generates the EHTv3 cryptographic signature for a given message.
 * The signature is encoded in the 'sm' array, and the length of the signature is stored in 'smlen'.
 *
 * @param sm A pointer to the signed message.
 * @param smlen A pointer to the length of the signed message.
 * @param m A pointer to the message.
 * @param mlen The length of the message.
 * @param sk A pointer to the secret key.
 * @return 0 for successful execution and -2 if memory allocation fails.
 */
int sig_gen(unsigned char *sm, unsigned long long* smlen, const unsigned char* m, unsigned long long mlen, const unsigned char* sk)
{
	// *** peak memory estimate of sig_gen (v4l5): 37.2 kilobytes ***
	
	// Initialize the rng
	randombytes_init((unsigned char*)sk, NULL, 256);
	
	// Declare the variables that will be allocated memory in the function
	unsigned short*** C = NULL;
	unsigned short*** C1 = NULL;
	unsigned short*** C1invh = NULL;
	unsigned short*** C1invC2 = NULL;
	unsigned short*** T = NULL;
	unsigned short*** a = NULL;
	unsigned short*** x = NULL;
	unsigned short*** y = NULL;
	unsigned short*** z = NULL;
	unsigned short* B_u = NULL;
	unsigned short* B_x = NULL;
	unsigned short* B_y = NULL;
	unsigned short* B_z = NULL;
	unsigned short* product = NULL;

	// Matrix C Generation
	C = allocate_gr_matrix_memory(M, K*N);
	
	if(C==NULL)
	{
		goto cleanup;
	}
	
	zero_gr_matrix(M, M+1, C);
	
	// Populate C1
	for(int i=0; i<M; i++)
	{
		for(int j=0; j<M; j++)
		{
			if(i+j!=M-1)
			{
				// |c1_ij| = norm1
				int k = NIST_rng(R);
				C[i][j][k] = (NIST_rng(2)==0)?1:Q-1;
			}
			else
			{
				// |c1_ij| = norm2
				for(int s=0; s<NORM2; s++)
				{
					int k = NIST_rng(R);
					
					if(C[i][j][k]==0)
					{
						C[i][j][k] = (NIST_rng(2)==0)?1:Q-1;
					}
					else if(C[i][j][k]<NORM2)
					{
						C[i][j][k] = C[i][j][k] + 1;
					}
					else
					{
						C[i][j][k] = C[i][j][k] - 1;
					}
				}
			}
		}	
	}
	
	// Populate C2
	for(int i=0; i<M; i++)
	{
		for(int s=0; s<NORM2; s++)
		{
			int k = NIST_rng(R);
		
			if(C[i][M][k]==0)
			{
				C[i][M][k] = (NIST_rng(2)==0)?1:Q-1;
			}
			else if(C[i][M][k]<NORM2)
			{
				C[i][M][k] = C[i][M][k] + 1;
			}
			else
			{
				C[i][M][k] = C[i][M][k] - 1;
			}
		}
	}
	
	// Matrix T Generation
	T = allocate_gr_matrix_memory(K*N, N);
	
	if(T==NULL)
	{
		goto cleanup;
	}
	
	zero_gr_matrix(K*N, N, T);
	
	for(int j=0; j<N; j++)
	{
		for(int i=0; i<K; i++)
		{
			T[K*j+i][j][0] = TUPPLE[i];
		}
		
		for(int i=K*j+K; i<K*N; i++)
		{
			for(int k=0; k<R; k++)
			{
				T[i][j][k] = NIST_rng(Q);
			}
		}
	}
	
	// Get componenet of matrix B
	B_u = allocate_vector_memory(R);
	B_x = allocate_vector_memory(R);
	B_y = allocate_vector_memory(R);
	B_z = allocate_vector_memory(R);
	
	if(B_u==NULL || B_x==NULL || B_y==NULL || B_z==NULL)
	{
		goto cleanup;
	}
	
	// B_z
	for (int k = 0; k < R; k++)
	{
		B_z[k] = NIST_rng(Q);
	}

	// B_y
	for (int k = 0; k < R; k++)
	{
		B_y[k] = NIST_rng(Q);
	}

	// B_x
	for (int k = 0; k < R; k++)
	{
		B_x[k] = NIST_rng(Q);
	}

	// B_u
	for (int k = 0; k < R; k++)
	{
		B_u[k] = NIST_rng(Q);
	}
	
	// Getting C1inv*h and C1inv*C2 from h (HASH) and C2
	C1invh = allocate_gr_matrix_memory(M, 1); // h to become C1inv*h
	C1invC2 = allocate_gr_matrix_memory(M, 1); // C2 to become C1inv*C2
	C1 = allocate_gr_matrix_memory(M, M); // to become identity to achieve the above 2
	
	if(C1invh==NULL || C1invC2==NULL || C1==NULL)
	{
		goto cleanup;
	}
	
	// HASH of Message
	hash_of_message(m, mlen, C1invh); // h storeed in C1invh as it will tranform to C1inv*h in the next steps
	
	// C2
	for(int i=0; i<M; i++)
	{
		for(int k=0; k<R; k++)
		{
			C1invC2[i][0][k] = C[i][M][k]; // C2 storeed in C1invC2 as it will tranform to Cinv*C2 in the next steps
		}
	}
	
	// C1
	for(int i=0; i<M; i++)
	{
		for(int j=0; j<M; j++)
		{
			for(int k=0; k<R; k++)
			{
				C1[i][j][k] = C[i][j][k];
			}
		}
	}
	
	// Precompute C1invh (C1inv*h) and C1invC2 (C1inv*C2) for the next steps
	solve_C1invh_and_C1invC2(C1, C1invh, C1invC2, sk);
	
	free_gr_matrix(M, M, C1); C1 = NULL;
	
	// Find a valid signature
	a = allocate_gr_matrix_memory(K*N, 1);
	y = allocate_gr_matrix_memory(N, 1);
	z = allocate_gr_matrix_memory(K*N, 1);
	product = allocate_vector_memory(R); // temp variable to hold a product
	
	if(a==NULL || y==NULL || z==NULL || product==NULL)
	{
		goto cleanup;
	}
	
	// Here we randomize tail entries of 'a' and solve for the remaing of 'a' and then for 'z' 
	// from these we can check if sufficient (L) values from e = C*z are within the bound S
	int within_bound = 0;
	while(within_bound<L)
	{
		within_bound = 0;
		
		solve_a(C1invh, C1invC2, a);
		solve_z(T, a, y, z);
		
		for(int i=0; i<M; i++)
		{
			zero_vector(R, product);
			
			for(int j=0; j<K*N; j++)
			{
				product_in_Gq(C[i][j], z[j][0], product, 0);
			}
			
			for(int k=0; k<R; k++)
			{
				int e = product[k];
				
				if(e<=S || e>=Q-S)
				{
					within_bound++;
				}
			}
		}
	}
	
	// We no longer need the following
	free_gr_matrix(M, K*N, C); C = NULL;
	free_gr_matrix(K*N, N, T); T = NULL;
	free_gr_matrix(M, 1, C1invh); C1invh = NULL;
	free_gr_matrix(M, 1, C1invC2); C1invC2 = NULL;
	free_gr_matrix(K*N, 1, a); a = NULL;
	free_gr_matrix(K*N, 1, z); z = NULL;
	free(product); product = NULL;
	
	// Compute x (signature)
	x = allocate_gr_matrix_memory(N, 1);
	
	if(x==NULL)
	{
		goto cleanup;
	}
	
	
	// x is computed in this function:
	compute_x(B_u, B_x, B_y, B_z, y, x);
	
	free(B_u); B_u = NULL;
	free(B_x); B_x = NULL;
	free(B_y); B_y = NULL;
	free(B_z); B_z = NULL;
	free_gr_matrix(N, 1, y); y = NULL;
	
	// Store m and x in sm and update smlen
	mx_to_sm(m, mlen, x, sm, smlen);
	
	free_gr_matrix(N, 1, x); x = NULL;
	
	return 0;
	
	////////////////////////////////////
	cleanup:
		// Free all the memory we may have allocated.
		free_gr_matrix(M, K*N, C); C = NULL;
		free_gr_matrix(M, M, C1); C1 = NULL;
		free_gr_matrix(M, 1, C1invh); C1invh = NULL;
		free_gr_matrix(M, 1, C1invC2); C1invC2 = NULL;
		free_gr_matrix(K*N, N, T); T = NULL;
		free_gr_matrix(K*N, 1, a); a = NULL;
		free_gr_matrix(N, 1, x); x = NULL;
		free_gr_matrix(N, 1, y); y = NULL;
		free_gr_matrix(K*N, 1, z); z = NULL;
		free(B_u); B_u = NULL;
		free(B_x); B_x = NULL;
		free(B_y); B_y = NULL;
		free(B_z); B_z = NULL;
		free(product); product = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////	
}

