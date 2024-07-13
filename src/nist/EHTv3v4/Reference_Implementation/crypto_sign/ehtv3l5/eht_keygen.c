#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "general_functions.h"
#include "general_functions_with_tables.h"


/**
 * This function converts matrix A into the public key pk.
 * It does this by first mapping each index in A to a binary representation, then concatenating these binary representations into bytes.
 *
 * @param A A pointer to the matrix that will be converted.
 * @param pk A pointer to the public key where the result will be stored.
 */
void A_to_pk(unsigned char **A, unsigned char *pk)
{
    // Number of bits needed to represent each value in A.
    // We need this because A contains values in the range [0, Q)
	int num_bits = (int)ceil(log2(Q));

    // We will use this array to store the binary representation of each possible value in A.
	bool index_in_bits[Q][num_bits];

    // Temporary array to store the binary representation of a number.
	bool temp_bits[num_bits];
	
    // Compute the binary representation of each possible value in A.
	for(int i=0; i<Q; i++)
	{
		decimal_to_binary(i, temp_bits, num_bits);
		
		for(int j=0; j<num_bits; j++)
		{
			index_in_bits[i][j] = temp_bits[j];
		}
	}
	
    // Total number of bytes needed to store A.
	int A_byte_size = (int)ceil(M*N*num_bits/8);

    // Indices used to iterate over the values in A and their binary representations.
	int i1 = 0;
	int j1 = 0;
	int k1 = 0;
	
    // Convert A into a byte array.
	for(int i=0; i<A_byte_size; i++)
	{
		pk[i] = 0;
		
		for(int j=0; j<8; j++)
		{
            // Each byte is filled with bits from the binary representations of values in A.
			pk[i] |= index_in_bits[A[i1][j1]][k1] << j;
			
			k1++;
			
            // Move to the next value in A once we have used all the bits from the current value.
	        if(k1==num_bits) 
	        {
	            k1 = 0;
	            
	            if(j1<N-1)
	            {
	                j1++;
	            }
	            else
	            {
	            	j1 = 0;
	                i1++;
	            }
	        }	
		}
	}
}

/**
 * This function stores the characteristic polynomial coefficients of matrix C1 to sk.
 *
 * @param C1cp A pointer to an array of characteristic polynomial coefficients of matrix C1
 * @param sk A pointer to an array where the secret key will be stored.
 */
void C1cp_to_sk(unsigned char* C1cp, unsigned char* sk)
{
	// Base of Q and its size, base of char and its size in terms of Q
	int base_Q = Q;
	int size_Q = M;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Initialize a portion of secret key to zeros
	for(int i=48; i<48+size_char; i++)
	{
		sk[i] = 0;
	}
	
	// Perform base conversion from base Q to base 256 (base of char)
	for(int i=0; i<M; i++)
	{
		int carry = C1cp[i];
		
		for(int j=48+size_char-1; j>=48; j--)
		{
			int temp = sk[j]*base_Q + carry;
            sk[j] = temp%base_char;
            carry = temp/base_char;
		}
	}
}

/**
 * This function generates the characteristic polynomial of matrix C1. According to Algorithm 2.2.9 from "A Course in Computational Algebraic Number Theory" by Henri Cohen.
 *
 * @param C Pointer to the matrix in which C1 is contained for which the characteristic polynomial is to be computed.
 * @param H Pointer to the Hessenberg matrix which will be filled during computation.
 * @param CP Pointer to the matrix where the characteristic polynomial coefficients will be stored.
 * @param C1cp Pointer to an array where the characteristic polynomial will be stored.
 * @return Returns a boolean indicating if C1 is invertible by checking if the first coefficient in the characteristic polynomial is non-zero.
 */
bool C1_characteristic_polynomial(unsigned char** C, unsigned char** H, unsigned char** CP, unsigned char* C1cp)
{
	// Copy matrix C into H. H will be transformed into a Hessenberg matrix.
	for(int i=0; i<M; i++)
	{
		for(int j=0; j<M; j++)
		{
			H[i][j] = C[i][j];
		}
	}
	
	// Convert matrix H into a Hessenberg matrix
	for(int m=1; m<M-1; m++)
	{
		bool check = 0;
		int i;
		
		// Search for first non-zero element in the (m-1)th column starting from row m+1
		for(i=m+1; i<M; i++)
		{
			if(H[i][m-1]!=0)
			{
				check = 1;
				break;
			}
		}
		
		if(check==1) // if a non-zero element was found
		{
			if(H[m][m-1]!=0) // if the element in position (m, m-1) is non-zero
			{
				i = m;
			}
			
			int t = H[i][m-1];
			
			// Swap the i-th row and the m-th row
			if(i!=m)
			{
				for(int j=m-1; j<M; j++)
				{
					int temp = H[i][j];
					H[i][j] = H[m][j];
					H[m][j] = temp;
				}
				
				// Swap the i-th column and the m-th column
				for(int j=0; j<M; j++)
				{
					int temp = H[j][i];
					H[j][i] = H[j][m];
					H[j][m] = temp;
				}
			}
			
			for(i=m+1; i<M; i++)
			{
				if(H[i][m-1]!=0)
				{
					int u = prod(H[i][m-1], inverse(t));
					
					// Update the i-th row
					for(int j=m-1; j<M; j++)
					{
						H[i][j] = sub(H[i][j], prod(u, H[m][j]));
					}
					
					// Update the m-th column
					for(int j=0; j<M; j++)
					{
						H[j][m] = add(H[j][m], prod(u, H[j][i]));
					}
				}
			}
		}
	}
	
	// Variables to hold the values of the polynomial
	unsigned char mul_x[M+1];
	unsigned char mul_c[M+1];
	
	// Initialize the polynomial coefficients to zero
	zero_matrix(M+1, M+1, CP);
	
	// The characteristic polynomial of a matrix is det(xI - A), where x is a variable,
	// I is the identity matrix, A is the matrix, and n is the order of the matrix.
	// The leading coefficient of the polynomial is set to (-1)^n. Since we're working
	// over a finite field of order Q, -1 is represented as Q-1, and so the leading
	// coefficient is 1 when n is even and Q-1 when n is odd.
	if(M%2==0)
	{
	    CP[0][0] = 1;
	}
	else
	{
	    CP[0][0] = Q-1;
	}
	
	// Determine characteristic polynomial coefficients
	for(int m=0; m<M; m++)
	{
		int c = H[m][m];
		
		zero_vector(M+1, mul_x);
		zero_vector(M+1, mul_c);
		
		// Multiply the current polynomial by x and by -c (in parallel)
		for(int i=0; i<m+1; i++)
		{
			mul_x[i+1] = CP[m][i];
			mul_c[i] = sub(0, prod(c, CP[m][i]));
		}
		
		// Add the two polynomials obtained in the previous step
		for(int i=0; i<=m+1; i++)
		{
			CP[m+1][i] = add(mul_x[i], mul_c[i]);
		}
		
		// Subtract the product of the Hessenberg coefficients and the previous polynomials
		int t=1;
		for(int i=0; i<m; i++)
		{
			t = prod(t, H[m-i][m-i-1]);
			
			for(int j=0; j<=m+1; j++)
			{
				CP[m+1][j] = sub(CP[m+1][j], prod(t, prod(H[m-i-1][m], CP[m-i-1][j])));
			}
		}
	}
	
	if(CP[M][0]==0)
	{
		return 0; // C1 is not invertible
	}
	else
	{
		for(int i=0; i<M+1; i++)
		{
			C1cp[i] = CP[M][i];
		}
		
		return 1; // C1 is invertiable and we have saved its characteristic polynonomial coefficient in C1cp
	}
}

/**
 * This function generates matrix C and a secret key sk.
 * The function first finds a suitable (invertible) matrix C1 (which is a part of matrix C), and then fills the rest of matrix C.
 *
 * @param C A pointer to the matrix where the result will be stored.
 * @param H A pointer to a matrix that becomes the Hessenberg Matrix form of C1
 * @param CP A pointer to the matrix for storing the characteristic polynomial.
 * @param sk A pointer to the secret key.
 */
void generate_C_sk(unsigned char** C, unsigned char** H, unsigned char** CP, unsigned char* sk)
{
    // C1_index is a helper matrix that stores indices for the non-zero elements of C1.
    // C1cp is an array that will hold the characteristic polynomial of C1.
    // tracker is a helper matrix used to track the chosen indices in the permutation process.
    unsigned short C1_index[M][NORM1];
    unsigned char C1cp[M+1];
    bool tracker[M][M];

    // The outer loop runs until a suitable C1 is found.
    // A suitable C1 should have a non-zero constant term in its first characteristic polynomial coefficient (indicating it is invertible).
    bool check1 = 0;
    while(check1==0)
    {
        // Initialize the random number generator
        for(int i=0; i<48; i++)
        {
            sk[i] = NIST_rng(256);
        }
        randombytes_init(sk, NULL, 256);

        // Initialize C1_index and tracker arrays
        // At first, C1_index is set to the identity matrix
        // And tracker is set to all zeros
        for(int i=0; i<M; i++)
        {
            for(int j=0; j<NORM1; j++)
            {
                C1_index[i][j] = i;
            }
            for(int j=0; j<M; j++)
            {
                tracker[i][j] = 0;
            }
        }

        // Generate a random permutation of the first column of C1_index (this is one permutation of the identity matrix)
        // and mark the chosen values in the tracker
        for(int i=0; i<M; i++)
        {
            int s = NIST_rng(M-i) + i;
            int t = C1_index[s][0];
            C1_index[s][0] =  C1_index[i][0];
            C1_index[i][0] = t;
            tracker[i][t] = 1;	
        }

        // Generate a random permutation for the rest of the columns of C1_index (to represent adding permutations of the identity matrix - like forming a latin rectangle)
        // In case of conflicts (the value is already chosen for this row), the permutation process restarts for its currrent column
        for(int j=1; j<NORM1; j++)
        {
            bool check2 = 0;

            while(check2==0)
            {
                check2 = 1;

                for(int i=0; i<M; i++)
                {
                    int s = NIST_rng(M-i) + i;
                    int t = C1_index[s][j];

                    if(tracker[i][t]==1)
                    {
                        check2 = 0;
                        for(int k=0; k<i; k++)
                        {
                            tracker[k][C1_index[k][j]] = 0;
                        }
                        break;
                    }

                    C1_index[s][j] =  C1_index[i][j];
                    C1_index[i][j] = t;
                    tracker[i][t] = 1;
                }
            }
        }

        // Initialize matrix C to all zeros
        zero_matrix(M, M+D, C);

        // Fill in matrix C with random non-zero values at positions defined by C1_index
        // The non-zero values are either 1 or Q-1 (Q-1 represents -1 as per the description)
        for(int i=0; i<M; i++)
        {
            for(int j=0; j<NORM1; j++)
            {
                C[i][C1_index[i][j]] = (NIST_rng(2)==0)?1:Q-1;
            }
        }

        // Check if C1 is invertible through its characteristic polynomial
        check1 = C1_characteristic_polynomial(C, H, CP, C1cp);
    }

    // Store the characteristic polynomial of C1 in the secret key
    C1cp_to_sk(C1cp, sk);

    // Fill the remaining columns of C with C2
	for(int i=0; i<M; i++)
	{
		for(int k=0; k<NORM2; k++)
		{
			int j = M + NIST_rng(D);
			
            // If the current value is 0, assign a random value
			if(C[i][j]==0)
			{
				C[i][j] = (NIST_rng(2)==0)?1:Q-1;
			}
            // If the current value is less than NORM2, increment it
			else if(C[i][j]<NORM2)
			{
				C[i][j] = C[i][j] + 1;
			}
            // Otherwise, decrement it
			else
			{
				C[i][j] = C[i][j] - 1;
			}
		}
	}
}

/**
 * This function calculates the inverse of a triangular matrix.
 * The type parameter defines whether the matrix is lower triangular (type=0) or upper triangular (type=1).
 *
 * @param n The number of rows (and columns) in the matrix.
 * @param A A pointer to the input triangular matrix.
 * @param Ainv A pointer to the matrix where the result (inverse of matrix A) will be stored.
 * @param type The type of triangular matrix (0 for lower triangular, 1 for upper triangular).
 */
void triangular_matrix_inverse(int n, unsigned char** A, unsigned char** Ainv, int type)
{
	int l, u;  // Auxiliary variables to work with the type of triangular matrix

	// Determine the type of triangular matrix
	if(type==0)
	{
		l = 1;  // For a lower triangular matrix, the lower part is processed
		u = 0;  // and the upper part is ignored
	}
	if(type==1)
	{
		l = 0;  // For an upper triangular matrix, the upper part is processed
		u = 1;  // and the lower part is ignored
	}

	// Initialize the resulting matrix (Ainv) as a zero matrix
	zero_matrix(n, n, Ainv);

	// The diagonal of the inverse of a triangular matrix is the inverse of the original diagonal
	for(int i=0; i<n; i++)
	{
		Ainv[i][i] = inverse(A[i][i]);  // Inverse each diagonal element
	}

	// Compute the off-diagonal elements of the inverse matrix
	for(int j=1; j<n; j++)
	{
		for(int i=0; i<n-j; i++)
	    {
	    	int psum = 0;  // Initialize partial sum
	    	int inv = inverse(A[i][i]);  // Inverse the diagonal element
	    	
	    	// Compute the partial sum
	    	for(int k=1; k<=j; k++)
	    	{
	    		// Add the product of corresponding elements from the original and inverse matrices
	    		psum = psum + A[i+k*l][i+k*u]*Ainv[i+j*l+k*u][i+j*u+k*l];
			}
			
	    	// Compute and assign the off-diagonal element of the inverse matrix
	    	Ainv[i+j*l][i+j*u] = mod_q(-1*inv*psum);
		}
	}
}

/**
 * This function generates a pair of public and private keys for EHTv3
 * The public key is calculated by using the formula A = C*T*Binv which is then stored in pk
 * The the private key is a composed of the seed for the RNG and the characteristic polynonomial coefficients of matrix C1
 *
 * @param pk A pointer to the public key where the result will be stored.
 * @param sk A pointer to the private key where the result will be stored.
 * @return 0 if the function was successful, -2 if memory allocation fails.
 */
int key_gen(unsigned char *pk, unsigned char *sk)
{
	// *** peak memory estimate of key_gen (v3l1): 769 kilobytes ***
	
    // Declare the variables that will be allocated memory in the function
	unsigned char** C;
	unsigned char** H;
	unsigned char** CP;
	unsigned char** T;
	unsigned char** TM;
	unsigned char** Linv;
	unsigned char** Uinv;
	unsigned char** Binv;
	unsigned char** TBinv;
	unsigned char** A;
	
	C = allocate_unsigned_char_matrix_memory(M, M+D);
	H = allocate_unsigned_char_matrix_memory(M, M);
	CP = allocate_unsigned_char_matrix_memory(M+1, M+1);
	
	if(C==NULL || H==NULL || CP==NULL)
	{
		goto cleanup;
	}
	
    // Generate the matrix C and the private key sk.
	generate_C_sk(C, H, CP, sk);
	
    // We no longer need the matrices H and CP.
	free_matrix(M, H); H = NULL;
	free_matrix(M+1, CP); CP = NULL;
	
    // Generate the matrix T.
	T = allocate_unsigned_char_matrix_memory(K*N, N);
	
	if(T==NULL)
	{
		goto cleanup;
	}
	
    // Initialize T to all zeros.
	zero_matrix(K*N, N, T);
	
	for(int i=0; i<K*N; i++)
	{
        // Fill in the lower triangular part of T with random numbers.
		for(int j=0; j<i/2; j++)
		{
			T[i][j] = NIST_rng(Q);
		}
		
        // The diagonal of T contains tuples.
		T[i][i/2] = TUPPLE[i%K];
	}
	
    // Generate the inverse of the matrix B.
	TM = allocate_unsigned_char_matrix_memory(N, N);
	Linv = allocate_unsigned_char_matrix_memory(N, N);
	Uinv = allocate_unsigned_char_matrix_memory(N, N);
	
	if(TM==NULL || Linv==NULL || Uinv==NULL)
	{
		goto cleanup;
	}
	
    // Initialize TM to a random lower triangular matrix.
	zero_matrix(N, N, TM);
	
	for(int i=0; i<N; i++)
	{
		TM[i][i] = 1 + NIST_rng(Q-1);

		for(int j=i+1; j<N; j++)
		{
			TM[j][i] = NIST_rng(Q);
		}
    }
    
    // Compute the inverse of TM, storing the result in Linv.
	triangular_matrix_inverse(N, TM, Linv, 0);
	
    // Initialize TM to a random upper triangular matrix.
	zero_matrix(N, N, TM);
	
	for(int i=0; i<N; i++)
	{
		TM[i][i] = 1 + NIST_rng(Q-1);
		
		for(int j=0; j<i; j++)
		{
			TM[j][i] = NIST_rng(Q);
		}
    }
	
    // Compute the inverse of TM, storing the result in Uinv.
	triangular_matrix_inverse(N, TM, Uinv, 1);
	
    // We no longer need the matrix TM.
    free_matrix(N, TM); TM = NULL;
	
	Binv = allocate_unsigned_char_matrix_memory(N, N);
	
	if(Binv==NULL)
	{
		goto cleanup;
	}
	
    // Compute Binv = Uinv*Linv.
	matrix_multiply(N, N, N, Uinv, Linv, Binv);
	
    // We no longer need the matrices Linv and Uinv.
	free_matrix(N, Linv); Linv = NULL;
	free_matrix(N, Uinv); Uinv = NULL;
	
    // Generate the matrix A.
	TBinv = allocate_unsigned_char_matrix_memory(K*N, N);
	
	if(TBinv==NULL)
	{
		goto cleanup;
	}
	
    // Compute TBinv = T*Binv.
	matrix_multiply(K*N, N, N, T, Binv, TBinv);
	
    // We no longer need the matrices T and Binv.
	free_matrix(K*N, T); T = NULL;
	free_matrix(N, Binv); Binv = NULL;
	
	A = allocate_unsigned_char_matrix_memory(M, N);
	
	if(A==NULL)
	{
		goto cleanup;
	}
	
    // Compute A = C*TBinv.
    matrix_multiply(M, K*N, N, C, TBinv, A);
    
    // We no longer need the matrices C and TBinv.
    free_matrix(M, C); C = NULL;
    free_matrix(K*N, TBinv); TBinv = NULL;
    
    // Store A into the public key.
    A_to_pk(A, pk);
    
    // We no longer need the matrix A.
    free_matrix(M, A); A = NULL;
    
    // The function was successful.
    return 0;
    
    ////////////////////////////////////
	cleanup:
		// Free all the memory we may have allocated.
		free_matrix(M, C); C = NULL;
		free_matrix(M, H); H = NULL;
		free_matrix(M+1, CP); CP = NULL;
		free_matrix(K*N, T); T = NULL;
		free_matrix(N, TM); TM = NULL;
		free_matrix(N, Linv); Linv = NULL;
		free_matrix(N, Uinv); Uinv = NULL;
		free_matrix(N, Binv); Binv = NULL;
		free_matrix(K*N, TBinv); TBinv = NULL;
		free_matrix(M, A); A = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////
}


