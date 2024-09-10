#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "general_functions.h"
#include "general_functions_with_tables.h"

/**
 * This function converts part of the secret key to the base of Q and stores it in C1cp which is the characteristic polynomial of matrix C1.
 *
 * @param sk A pointer to the secret key.
 * @param C1cp A pointer to the resulting vector.
 */
void sk_to_C1cp(const unsigned char* sk, unsigned char* C1cp)
{
    // Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = M;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Zero-out the C1cp vector
	zero_vector(M, C1cp);
	
	// Loop through part of the secret key, treating it as a number in base 256
	for(int j=48; j<48+size_char; j++)
    {
        // Current "carry" value is the j-th element of sk
        int carry = sk[j];
        
        // Convert the base 256 number to base Q and store it in C1cp
        for(int i=M-1; i>=0; i--)
        {
            // Calculate the new value and carry
            int temp = C1cp[i]*base_char + carry;
            C1cp[i] = temp%base_Q;
            carry = temp/base_Q;
        }
    }
    
    // Set the last element of C1cp to 1 as this was not stored and is the same for any characteristic polynomial
    C1cp[M] = 1;
}

/**
 * This function stores the message (m) and signature (x) to the signed message (sm).
 * The length of the signed message (smlen) is updated to that of the message length + the signature length
 *
 * @param m A pointer to the message.
 * @param mlen The length of the message.
 * @param x A 2D pointer to the signature x.
 * @param sm A pointer to the signed message.
 * @param smlen A pointer to the length of the signed message.
 */
void mx_to_sm(const unsigned char* m, unsigned long long mlen, unsigned char** x, unsigned char* sm, unsigned long long* smlen)
{
    // Define base and size parameters for Q and char
	int base_Q = Q;
	int size_Q = N;
	int base_char = 256;
	int size_char = (int)ceil(size_Q*log(base_Q)/log(base_char));
	
	// Zero-out the sm vector
	zero_vector(size_char, sm);
    
    // Convert each element of x from base Q to base 256
	for(int i=0; i<N; i++)
	{
		int carry = x[i][0];
			
		for(int j=size_char-1; j>=0; j--)
		{
            // Calculate the new value and carry
			int temp = sm[j]*base_Q + carry;
            sm[j] = temp%base_char;
            carry = temp/base_char;
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
 * This function generates the matrix C composed of C1 and C2
 *
 * @param C A 2D pointer to the matrix C.
 * @param C1_index A 2D pointer to the matrix C1_index which will hold the indices of non-zero elements of C.
 * @param C1_value A 2D pointer to the matrix C1_value which will hold the non-zero values of C.
 */
void generate_C(unsigned char** C, unsigned short** C1_index, unsigned char** C1_value)
{
    // Initialize the tracker matrix to keep track of non-zero elements in C
	bool tracker[M][M];
	
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
			int value = (NIST_rng(2)==0)?1:Q-1;
			C1_value[i][j] = value;
			C[i][C1_index[i][j]] = value;
		}
	}
	
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
 * The function randomizes tail (d) entries of `a` (`a2`) and solves for the top (m) portion of `a` (`a1`).
 * It used the characteristic polynomial in a set of operation that evaulate as an inverse to solve for `a1` in the following equation:  C1*a1 + C2*a2 = h
 *
 * @param C A 2D pointer to the matrix C.
 * @param C1_index A 2D pointer to the matrix C1_index which holds the indices of non-zero elements of C.
 * @param C1_value A 2D pointer to the matrix C1_value which holds the non-zero values of C.
 * @param C1cp A pointer to the characteristic polynomial of C1.
 * @param h A 2D pointer to `h`.
 * @param a A 2D pointer to `a`.
 */
void solve_a(unsigned char** C, unsigned short** C1_index, unsigned char** C1_value, unsigned char* C1cp, unsigned char** h, unsigned char** a)
{
	// The tail entries of `a` (`a2`) changes if condition max_l(e) <= s is not met 
	for(int i=0; i<D; i++)
    {
    	a[M+i][0] = NIST_rng(Q);
	}
	
	// `Cmh2` is used to store C^m*h2 and `last_Cmh2` is used to store C^(m-1)*h2
	unsigned char Cmh2[M];
	unsigned char last_Cmh2[M];
	
    // Intitialize `last_Cmh2`
    for(int i=0; i<M; i++)
    {
    	int sum = h[i][0];
    	
    	for(int j=0; j<D; j++)
    	{
    		sum = sum - C[i][M+j]*a[M+j][0];
		}
		
		last_Cmh2[i] = mod_q(sum);
	}
	
	// Compute the inverse of the first element of `C1cp` and negate it
	int neg_inv = -1*inverse(C1cp[0]);
	
	// Initialize `a1` using `C1cp` and `last_Cmh2`
	for(int i=0; i<M; i++)
	{
		a[i][0] = mod_q(neg_inv*C1cp[1]*last_Cmh2[i]);
	}
	
	// Compute `a1` in an interative manner
	for(int i=1; i<M; i++)
	{
		zero_vector(M, Cmh2);
		
		for(int j=0; j<M; j++)
		{
			int sum = 0;
			for(int k=0; k<NORM1; k++)
			{
				sum = sum + C1_value[j][k]*last_Cmh2[C1_index[j][k]];
			}
			
			Cmh2[j] = p_mod_q(sum);
		}
		
		for(int j=0; j<M; j++)
		{
			last_Cmh2[j] = Cmh2[j];
			a[j][0] = mod_q(a[j][0] + neg_inv*C1cp[i+1]*Cmh2[j]);
		}
	}
}

/**
 * This function solves for the vector `z` (containing values 'u') based on the algorithm described in the description at 1.3.2.
 *
 * @param T A 2D pointer to matrix `T`.
 * @param a A 2D pointer to matrix `a`.
 * @param y A 2D pointer to vector `y`.
 * @param z A 2D pointer to the vector `z` where the results will be stored.
 */
void solve_z(unsigned char** T, unsigned char** a, unsigned char** y, unsigned char** z)
{
	// modification required here if changing security parameter K
	
	for(int i=0; i<N; i++)
	{
		int t0 = 0;
		int t1 = 0;
		
		for(int j=0; j<i; j++)
		{
			t0 = t0 + (T[K*i+0][j]*y[j][0]);
			t1 = t1 + (T[K*i+1][j]*y[j][0]);
		}
		
		int ax = mod_q(a[K*i+1][0] - t1 - TUPPLE[1]*(a[K*i][0] - t0));
		ax = (ax<=Q/2)?ax:ax-Q;
		
		int y1 = ax/TUPPLE[1];
		int y2 = ax%TUPPLE[1];
		
		if(abs(y2)<=C)
		{
			z[K*i+1][0] = s_mod_q(y2);
			z[K*i][0] = s_mod_q(-1*y1);
		}
		else
		{
			z[K*i+1][0] = s_mod_q(y2 - signum(ax)*TUPPLE[1]);
			z[K*i][0] = s_mod_q(-1*(y1 + signum(ax)));
		}
		
		y[i][0] = mod_q(a[K*i][0] - t0 - z[K*i][0]); // u
	}
}

/**
 * This function generates the EHTv3 cryptographic signature for a given message.
 * The signature is encoded in the 'sm' array, and the length of the signature is stored in 'smlen'.
 *
 * @param sm A pointer to the array where the signature will be stored.
 * @param smlen A pointer to the variable where the length of the signature will be stored.
 * @param m A pointer to the message.
 * @param mlen The length of the message.
 * @param sk A pointer to the secret key.
 * @return 0 for successful execution and -2 if memory allocation fails.
 */
int sig_gen(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk)
{
	// *** peak memory estimate of sig_gen (v3l1): 529 kilobytes ***
	
	// Initialize the rng
	randombytes_init((unsigned char*)sk, NULL, 256);
	
	// Declare the variables that will be allocated memory in the function
	unsigned char** C;
	unsigned short** C1_index;
	unsigned char** C1_value;
	unsigned char** T;
	unsigned char** B;
	unsigned char** LM;
	unsigned char** UM;
	unsigned char** h;
	unsigned char* C1cp;
	unsigned char** y;
	unsigned char** a;
	unsigned char** z;
	unsigned char** x;
	
	// Generate the matrix C
	C = allocate_unsigned_char_matrix_memory(M, M+D);
	C1_index = allocate_unsigned_short_matrix_memory(M, NORM1);
	C1_value = allocate_unsigned_char_matrix_memory(M, NORM1);
	
	if(C==NULL || C1_index==NULL || C1_value==NULL)
	{
		goto cleanup;
	}
	
	generate_C(C, C1_index, C1_value);
	
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
	
	// Matrix B Generation from LM and UM (lower and upper triangular matrix construction)
	B = allocate_unsigned_char_matrix_memory(N, N);
	LM = allocate_unsigned_char_matrix_memory(N, N);
	UM = allocate_unsigned_char_matrix_memory(N, N);
	
	if(B==NULL || LM==NULL || UM==NULL)
	{
		goto cleanup;
	}
	
	zero_matrix(N, N, LM);
	
	for(int i=0; i<N; i++)
	{
		LM[i][i] = 1 + NIST_rng(Q-1);

		for(int j=i+1; j<N; j++)
		{
			LM[j][i] = NIST_rng(Q);
		}
    }
    
	zero_matrix(N, N, UM);
	
	for(int i=0; i<N; i++)
	{
		UM[i][i] = 1 + NIST_rng(Q-1);
		
		for(int j=0; j<i; j++)
		{
			UM[j][i] = NIST_rng(Q);
		}
    }
    
    // Compute B = LM*UM
    matrix_multiply(N, N, N, LM, UM, B); 
	
	// We no longer need the matrices LM and UM.
	free_matrix(N, LM); LM = NULL;
	free_matrix(N, UM); UM = NULL;
	
	// HASH of Message
	h = allocate_unsigned_char_matrix_memory(M, 1);
	
	if(h == NULL)
	{
		goto cleanup;
	}
	
	hash_of_message(m, mlen, h);
	
	// Get the characteristic polynomial of C1
	C1cp = allocate_unsigned_char_vector_memory(M+1);
	
	if(C1cp == NULL)
	{
		goto cleanup;
	}
	
	sk_to_C1cp(sk, C1cp);
	
	// Find a valid signature
	y = allocate_unsigned_char_matrix_memory(N, 1);
	a = allocate_unsigned_char_matrix_memory(M+D, 1);
	z = allocate_unsigned_char_matrix_memory(K*N, 1);
	
	if(y == NULL || a == NULL || z == NULL)
	{
		goto cleanup;
	}
	
	// Here we randomize tail entries of 'a' and solve for the remaing of 'a' and then for 'z' 
	// from these we can check if sufficient (L) values from e = C*z are within the bound S
	int within_bound = 0;
	while(within_bound<L)
	{
		within_bound = 0;
		
		solve_a(C, C1_index, C1_value, C1cp, h, a);
		solve_z(T, a, y, z);
		
		for(int i=0; i<M; i++)
		{
			int e = 0;
			
			for(int j=0; j<M+D; j++)
			{
				e = e + C[i][j] * z[j][0];
			}
			
			e = p_mod_q(e);
			
			if(e <= S || e >= Q - S)
			{
				within_bound++;
			}
		}
	}
	
	// We no longer need the following
	free_matrix(M, C); C = NULL;
	free_short_matrix(M, C1_index); C1_index = NULL;
	free_matrix(M, C1_value); C1_value = NULL;
	free_matrix(K*N, T); T = NULL;
	free_matrix(M, h); h = NULL;
	free(C1cp); C1cp = NULL;
	free_matrix(M+D, a); a = NULL;
	free_matrix(K*N, z); z = NULL;
	
	// Compute x (signature)
	x = allocate_unsigned_char_matrix_memory(N, 1);
	
	if(x == NULL)
	{
		goto cleanup;
	}
	
	// x = B*y
	matrix_multiply(N, N, 1, B, y, x);
	
	free_matrix(N, B); B = NULL;
	free_matrix(N, y); y = NULL;
	
	// Store m and x in sm and update smlen
	mx_to_sm(m, mlen, x, sm, smlen);
	
	free_matrix(N, x); x = NULL;
	
	return 0;
	
	////////////////////////////////////
	cleanup:
		// Free all the memory we may have allocated.
		free_matrix(M, C); C = NULL;
		free_short_matrix(M, C1_index); C1_index = NULL;
		free_matrix(M, C1_value); C1_value = NULL;
		free_matrix(K*N, T); T = NULL;
		free_matrix(N, B); B = NULL;
		free_matrix(N, LM); LM = NULL;
		free_matrix(N, UM); UM = NULL;
		free_matrix(M, h); h = NULL;
		free(C1cp); C1cp = NULL;
		free_matrix(N, y); y = NULL;
		free_matrix(M+D, a); a = NULL;
		free_matrix(K*N, z); z = NULL;
		free_matrix(N, x); x = NULL;
		
		// The function failed to allocate memory at some point
		return -2;
	////////////////////////////////////
}



