#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "common_functions.h"

/**
 * This function converts inversions into a part of the secret key (after the 48 byte seed)
 * It performs conversion using base and size parameters for the inversion and char values.
 *
 * @param inversions A pointer to the inversions to be converted.
 * @param sk A pointer to the secret key where the result will be stored.
 */
void inversions_to_sk(unsigned short* inversions, unsigned char* sk)
{
	// Compute the base and size for inversion and char values
    // size_char calculates the number of characters needed to represent Q in base 256
	int base_Q = Q;
	int size_Q = 2+2*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Initialize the section of the secret key (that will store the inversions) to zeros
	for(int i=48; i<48+size_char; i++)
	{
		sk[i] = 0;
	}
	
	// Process each inversion, converting it to base 256 and storing it in the secret key
    // This is done by repeatedly dividing the inversion by 256 and storing the remainder in the secret key
    // This process is carried out from the least significant digit to the most significant
	for(int i=0; i<size_Q; i++)
	{
		int carry = inversions[i];
				
		for(int l=48+size_char-1; l>=48; l--)
		{
			int temp = sk[l]*base_Q + carry;
            sk[l] = temp%base_char;
            carry = temp/base_char;
		}
	}
}

/**
 * This function stores a group ring matrix A as bytes into the public key.
 * It performs base conversion from Q to 256
 *
 * @param A A pointer to the group ring matrix to be converted.
 * @param pk A pointer to the public key where the result will be stored.
 */
void A_to_pk(unsigned short*** A, unsigned char* pk)
{
	// Compute the base and size for the matrix and char values
    // size_char calculates the number of characters needed to represent the matrix in base 256
	int base_Q = Q;
	int size_Q = M*N*R;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Initialize the public key to zeros
	for(int i=0; i<size_char; i++)
	{
		pk[i] = 0;
	}
    
    // Process each coefficient of the group ring matrix, converting it to base 256 and storing it in the public key
    // This is done by repeatedly dividing the coefficient by 256 and storing the remainder in the public key
    // This process is carried out from the least significant digit to the most significant
	for(int i=0; i<M; i++)
	{
		for(int j=0; j<N; j++)
		{
			for(int k=0; k<R; k++)
			{
				int carry = A[i][j][k];
				
				for(int l=size_char-1; l>=0; l--)
				{
					int temp = pk[l]*base_Q + carry;
		            pk[l] = temp%base_char;
		            carry = temp/base_char;
				}
			}
		}
	}
}

/**
 * This function checks the invertibility of a group ring matrix C1 which is a part of a larger group ring matrix C.
 * It does so by trying to find a valid inverse for each diagonal elements when bringing it to row echelon form.
 * If a valid inverse is found for every diagonal element, the function concludes that C1 is invertible.
 *
 * @param C1 A pointer to the group ring matrix to be checked.
 * @param sk A pointer to the secret key.
 * @return Returns true if C1 is invertible, false otherwise.
 */
bool check_C1_invertibilty(unsigned short*** C1, unsigned char* sk)
{
	// Initialize arrays for holding the inversions, inverse, product and pivot
	unsigned short inversions[2+2*R];
	unsigned short inverse[R];
	unsigned short product[R];
	unsigned short pivot[R];
	
	// Go through each row of the matrix
	for(int i=0; i<M; i++)
	{	
		bool check_empty = 1;
		
		// Check if a non-zero element is or can be brought to the ith-diagonal
		for(int s=i; s<M; s++)
		{
			check_empty = is_gr_empty(C1[s][i]);
			
			if(check_empty==0)
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
				}
				
				break;
			}
		}
		
		if(check_empty==1)
		{
			return 0; // inverse does not exist
		}
		
		// Attempt to find an inverse for the current (ith) diagonal element
        // If an inverse does not exist, the matrix is not invertible, so return false
		if(inverse_in_Gq(C1[i][i], inverse)==0)
		{
			return 0; // inverse does not exist
		}
		
		// Store the inverse in the inversions array
		// The first inversion (i==0) is simple so we only store a single index and value
		// For the remaining inversions we store the entire inverse vec
		if(i==0)
		{
			for(int k=0; k<R; k++)
			{
				if(inverse[k]!=0)
				{
					inversions[0] = k;
					inversions[1] = inverse[k];
				}
			}
		}
		else
		{
			for(int k=0; k<R; k++)
			{
				inversions[2+R*(i-1)+k] = inverse[k];
			}
		}
		
		// If this is the last row, store the inversions to the secret key and return true (C1 is invertible)
		if(i==M-1)
		{
			inversions_to_sk(inversions, sk);
			
			return 1; // We now know that C1 is invertible
		}
		
		
		// Multiply the inverse with the elements of the current row
		C1[i][i][0] = 1;
		for(int k=1; k<R; k++)
		{
			C1[i][i][k] = 0;
		}
		
		for(int j=i+1; j<M; j++)
		{
			if(is_gr_empty(C1[i][j])==0)
			{
				product_in_Gq(inverse, C1[i][j], product, 1);
				
				for(int k=0; k<R; k++)
				{
					C1[i][j][k] = product[k];
				}
			}
		}
		
		// Use the current diagonal element (and row) to make all elements below it in the same column to be zero
		for(int j=i+1; j<M; j++)
		{
			if(is_gr_empty(C1[j][i])==0)
			{
				for(int k=0; k<R; k++)
				{
					pivot[k] = C1[j][i][k];
				}
				
				for(int s=0; s<M; s++)
				{
					if(is_gr_empty(C1[i][s])==0)
					{
						product_in_Gq(pivot, C1[i][s], product, 1);
					
						for(int k=0; k<R; k++)
						{
							C1[j][s][k] = s_mod_q(C1[j][s][k] - product[k]);
						}
					}
				}
			}
		}
	}
}

/**
 * This function generates matrix C accordind to the description and with it the secret key.
 *
 * @param C A pointer to the group ring matrix where the result will be stored.
 * @param sk A pointer to the secret key where the seed and inversions will be stored.
 */
void generate_C_sk(unsigned short*** C, unsigned char* sk)
{
	// Try to find a seed that produces an invertible C1
    // This is done by repeatedly generating random matrices C1 and checking their invertibility
    // If an invertible C1 is found, stop the loop
	bool check = 0;
	while(check==0)
	{
		check = 1;
		
		for(int i=0; i<48; i++)
		{
			sk[i] = NIST_rng(256);
		}
		
		randombytes_init(sk, NULL, 256); 
		
		zero_gr_matrix(M, K*N, C);
		
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
		
		// Check C1 invertibility
		check = check_C1_invertibilty(C, sk);
	}
	
	// Reinitialize the random number generator with the found seed and repopulate C1 as before
	randombytes_init(sk, NULL, 256); 	
	zero_gr_matrix(M, M+1, C);
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
}


/**
 * This function generates a pair of public and private keys for EHTv4
 * The public key is calculated by using the formula A = C*T*Binv which is then stored in pk
 * The the private key (sk) is a composed of the seed for the RNG and the inversions of matrix C1
 *
 * @param pk A pointer to the public key where the result will be stored.
 * @param sk A pointer to the private key where the result will be stored.
 * @return 0 if the function was successful, -2 if memory allocation fails.
 */
int key_gen(unsigned char* pk, unsigned char* sk)
{
	// *** peak memory estimate of key_gen (v4l1): 66.4 kilobytes ***
	
	// Declare the variables that will be allocated memory in the function
	unsigned short*** C = NULL;
	unsigned short*** T = NULL;
	unsigned short*** A = NULL;
	unsigned short*** tempA = NULL;
	unsigned short*** tempB = NULL;
    
	// Matrix C Generation from a suitable rngseed (sk)
	C = allocate_gr_matrix_memory(M, K*N);
	
	if(C==NULL)
	{
		goto cleanup;
	}
	
	// Generate the matrix C and the private key sk.
	generate_C_sk(C, sk);
	
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
	
	// Compute C*T (part of A)
	A = allocate_gr_matrix_memory(M, N);
	
	if(A==NULL)
	{
		goto cleanup;
	}
	
	grmatrix_multiply(M, K*N, N, C, T, A);
	
	free_gr_matrix(M, K*N, C); C = NULL;
	free_gr_matrix(K*N, N, T); T = NULL;
	
	// Compute A = CT*Binv components (as in the description)
	tempA = allocate_gr_matrix_memory(M, N);
	tempB = allocate_gr_matrix_memory(N, N);
	
	if(tempA==NULL || tempB==NULL)
	{
		goto cleanup;
	}
	
	// Negative B_z
	zero_gr_matrix(N, N, tempB);
	tempB[0][1][0] = 1;
	tempB[1][0][0] = 1;
	for(int k=0; k<R; k++)
	{
		tempB[1][1][k] = Q - NIST_rng(Q);
	}
	grmatrix_multiply(M, N, N, A, tempB, tempA);
	
	// Negative B_y
	zero_gr_matrix(N, N, tempB);
	tempB[0][0][0] = 1;
	tempB[1][1][0] = 1;
	for(int k=0; k<R; k++)
	{
		tempB[1][0][k] = Q - NIST_rng(Q);
	}
	grmatrix_multiply(M, N, N, tempA, tempB, A);
	
	// Negative B_x
	zero_gr_matrix(N, N, tempB);
	tempB[0][0][0] = 1;
	tempB[1][1][0] = 1;
	for(int k=0; k<R; k++)
	{
		tempB[0][1][k] = Q - NIST_rng(Q);
	}
	grmatrix_multiply(M, N, N, A, tempB, tempA);
	
	// Negative B_u
	zero_gr_matrix(N, N, tempB);
	tempB[0][1][0] = 1;
	tempB[1][0][0] = 1;
	for(int k=0; k<R; k++)
	{
		tempB[1][1][k] = Q - NIST_rng(Q);
	}
	grmatrix_multiply(M, N, N, tempA, tempB, A);
	
	free_gr_matrix(M, N, tempA); tempA = NULL;
	free_gr_matrix(N, N, tempB); tempB = NULL;
	
	// Store A into the public key.
	A_to_pk(A, pk);
	
	free_gr_matrix(M, N, A); A = NULL;
	
	return 0;
	
	////////////////////////////////////
	cleanup:
		// Free all the memory we may have allocated.
		free_gr_matrix(M, K*N, C); C = NULL;
		free_gr_matrix(K*N, N, T); T = NULL;
		free_gr_matrix(M, N, A); A = NULL;
		free_gr_matrix(N, N, tempA); tempA = NULL;
		free_gr_matrix(N, N, tempB); tempB = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////
}

















