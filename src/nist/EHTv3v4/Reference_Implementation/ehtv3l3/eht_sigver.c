#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "general_functions.h"

/**
 * This function gets the signature 'x' and message 'm' out of signed message `sm`.
 *
 * @param sm The signed message.
 * @param smlen The length of the signed message.
 * @param m The output byte array representing the message.
 * @param mlen A pointer to the length of message.
 * @param x A pointer to the signature.
 */
void sm_to_mx(const unsigned char* sm, unsigned long long smlen, unsigned char* m, unsigned long long* mlen, unsigned char** x)
{   
	// Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = N;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	zero_matrix(N, 1, x);
	
	// Convert each element of sm that stores x from base 256 to base Q
	for(int j=0; j<size_char; j++)
    {
        int carry = sm[j];
        
        for(int i=N-1; i>=0; i--)
        {
            int temp = x[i][0]*base_char + carry;
            x[i][0] = temp%base_Q;
            carry = temp/base_Q;
        }
    }
    
    // Update the length of the message
    *mlen = smlen - size_char;
    
    
    // Store the message in m
    for(int i=0; i<*mlen; i++)
	{
		m[i] = sm[size_char+i];
	}
}

/**
 * This function lets us retrive matrix A from the public key pk.
 * It does this by reversing what the function A_to_pk did in key_gen
 *
 * @param pk A pointer to the public key which contains a compressed version of A.
 * @param A A 2D pointer to the matrix that holds matrix A.
 */
void pk_to_A(const unsigned char *pk, unsigned char **A)
{   
	// Number of bits needed to represent each value in A.
    // We need this because we need to extract this number of consecutive bits from pk to form each consecutive value of A.
	int num_bits = (int)ceil(log2(Q));
	int i1 = 0;
	int j1 = 0;
	
	for(int i=0; i<M; i++)
	{
	    for(int j=0; j<N; j++)
	    {
	    	A[i][j] = 0;
	    	
	        for(int k=0; k<num_bits; k++)
	        {
	        	A[i][j] <<= 1;
	        	A[i][j] |= (pk[i1] >> j1) & 1;
	        	
	        	j1++;
	        	if(j1==8)
	        	{
	        		j1 = 0;
	        		i1++;
				}
			}
	    }
	}
}

/**
 * Performs signature verification.
 *
 * @param m The input byte array representing the message.
 * @param mlen A pointer to the length of the message.
 * @param sm The input byte array representing the signed message.
 * @param smlen The length of the signed message.
 * @param pk The input byte array representing the public key.
 * @return 0 if the verification is successful, -1 if unsuccessful, -2 if memory allocation failure.
 */
int sig_ver(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk)
{
	// *** peak memory estimate of sig_ver (v3l3): 449 kilobytes ***
	
	// Declare the variables that will be allocated memory in the function
	unsigned char** A;
	unsigned char** x;
	unsigned char** Ax;
	unsigned char** h;
	
	// Get A and x from pk and sk
	A = allocate_unsigned_char_matrix_memory(M, N);
	x = allocate_unsigned_char_matrix_memory(N, 1);
	
	if(A==NULL || x==NULL)
	{
		goto cleanup;
	}
	
    pk_to_A(pk, A);
	sm_to_mx(sm, smlen, m, mlen, x);
	
	// Get Ax
	Ax = allocate_unsigned_char_matrix_memory(M, 1);
	
	if(Ax==NULL)
	{
		goto cleanup;
	}
	
	// Ax = A*x
	matrix_multiply(M, N, 1, A, x, Ax); // A*x
	
	// Free A and x as we have Ax
	free_matrix(M, A); A = NULL;
	free_matrix(N, x); x = NULL;
	
	// HASH of Message
	h = allocate_unsigned_char_matrix_memory(M, 1);
	
	if(h==NULL)
	{
		goto cleanup;
	}
	
	hash_of_message(m, *mlen, h);
	
	// Verify the signature by checking how many values of e = h - A*x are within the bound S
	int within_bound = 0;
	for(int i=0; i<M; i++)
	{
		int e = s_mod_q(h[i][0] - Ax[i][0]);
			
		if(e<=S || e>=Q-S)
		{
			within_bound++;
		}
	}
	
	// Free h and Ax as we are done
	free_matrix(M, h); h = NULL;
	free_matrix(M, Ax); Ax = NULL;
	
	if(within_bound>=L)
	{
		return 0; // Verification Successfull
	}
	else
	{
		return -1; // Verification Unsuccessfull	
	}
	
	////////////////////////////////////
	cleanup:
		// Free all the memory we may have allocated.
		free_matrix(M, A); A = NULL;
		free_matrix(M, x); x = NULL;
		free_matrix(M, Ax); Ax = NULL;
		free_matrix(M, h); h = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////
}


