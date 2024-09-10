#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "common_functions.h"

/**
 * This function lets us retrive group ring matrix A from the public key pk.
 * It does this by reversing what the function A_to_pk did in key_gen (base conversion from 256 to Q)
 *
 * @param pk A pointer to the public key which contains a compressed version of A.
 * @param A A 3D pointer to the group ring matrix that holds matrix A.
 */
void pk_to_A(const unsigned char* pk, unsigned short*** A)
{
	// Define base and size parameters for Q and char
    int base_Q = Q;
	int size_Q = M*N*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	zero_gr_matrix(M, N, A);
	
	// Convert each element of pk (starting from the least significant byte) from base 256 to base Q
	for(int l=0; l<size_char; l++)
    {
        int carry = pk[l];
        
        for(int i=M-1; i>=0; i--)
        {
            for(int j=N-1; j>=0; j--)
            {
                for(int k=R-1; k>=0; k--)
                {
                    int temp = A[i][j][k]*base_char + carry;
                    A[i][j][k] = temp%base_Q;
                    carry = temp/base_Q;
                }
            }
        }
    }
}

/**
 * This function gets the signature 'x' and message 'm' out of signed message `sm`.
 *
 * @param sm The signed message.
 * @param smlen The length of the signed message.
 * @param m The output byte array representing the message.
 * @param mlen A pointer to the length of message.
 * @param x A pointer to the signature.
 */
void sm_to_mx(const unsigned char* sm, unsigned long long smlen, unsigned char* m, unsigned long long* mlen, unsigned short*** x)
{   
	// Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = N*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	zero_gr_matrix(N, 1, x);
	
	// Convert each element of sm that stores x from base 256 to base Q
	for(int l=0; l<size_char; l++)
    {
        int carry = sm[l];
        
        for(int i=N-1; i>=0; i--)
        {
            for(int k=R-1; k>=0; k--)
            {
                int temp = x[i][0][k]*base_char + carry;
                x[i][0][k] = temp%base_Q;
                carry = temp/base_Q;
            }
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

int sig_ver(unsigned char* m, unsigned long long* mlen, const unsigned char* sm, unsigned long long smlen, const unsigned char* pk)
{
	// *** peak memory estimate of sig_ver (v4l5): 15.2 kilobytes ***
	
	// Declare the variables that will be allocated memory in the function
	unsigned short*** A = NULL;
	unsigned short*** x = NULL;
	unsigned short*** Ax = NULL;
	unsigned short*** h = NULL;
	
	// Get A and x from pk and sk
	A = allocate_gr_matrix_memory(M, N);
	x = allocate_gr_matrix_memory(N, 1);
	
	if(A==NULL || x==NULL)
	{
		goto cleanup;
	}
	
	pk_to_A(pk, A);
	sm_to_mx(sm, smlen, m, mlen, x);
	
	// Ax = A*x
	Ax = allocate_gr_matrix_memory(M, 1);
	
	if(Ax==NULL)
	{
		goto cleanup;
	}
	
	grmatrix_multiply(M, N, 1, A, x, Ax); // A*x
	
	free_gr_matrix(M, N, A); A = NULL;
	free_gr_matrix(N, 1, x); x = NULL;
	
	// HASH of Message
	h = allocate_gr_matrix_memory(M, 1);
	
	if(h==NULL)
	{
		goto cleanup;
	}
	
	hash_of_message(m, *mlen, h);
	
	// Verify the signature by checking how many values of e = h - A*x are within the bound S
	int within_bound = 0;
	for(int i=0; i<M; i++)
	{
		for(int k=0; k<R; k++)
		{
			int e = s_mod_q(h[i][0][k] - Ax[i][0][k]);
			
			if(e<=S || e>=Q-S)
			{
				within_bound++;
			}
		}
	}
	
	// Free h and Ax as we are done
	free_gr_matrix(M, 1, Ax); Ax = NULL;
	free_gr_matrix(M, 1, h); h = NULL;
	
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
		free_gr_matrix(M, N, A); A = NULL;
		free_gr_matrix(M, M, x); x = NULL;
		free_gr_matrix(M, 1, Ax); Ax = NULL;
		free_gr_matrix(M, 1, h); h = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////
}

