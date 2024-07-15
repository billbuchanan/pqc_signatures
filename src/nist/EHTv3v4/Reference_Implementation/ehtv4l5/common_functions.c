#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "tables.h"
#include "keccak.h"

/**
 * This function allocates memory for an array of unsigned shorts.
 * 
 * @param n The length of the array to be allocated.
 * @return A pointer to the first element of the newly allocated array.
 *         If the allocation fails, NULL is returned.
 */
unsigned short* allocate_vector_memory(int n)
{
	unsigned short* A = malloc(n*sizeof(unsigned short));
    return A;
}

/**
 * This function allocates memory for a 3D matrix of unsigned shorts.
 * This is a group ring matrix where each element in the matrix hold an array of length R (the size of the group)
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @return A pointer to the first element of the newly allocated 3D matrix.
 *         If the allocation fails at any point, any memory that was 
 *         successfully allocated is freed and NULL is returned.
 */
unsigned short*** allocate_gr_matrix_memory(int m, int n)
{
    unsigned short*** A = malloc(m*sizeof(unsigned short**));
    if(A==NULL)
	{
        return NULL; // allocation failed
    }
    
    for(int i=0; i<m; i++)
	{
        A[i] = malloc(n*sizeof(unsigned short*));
        
        if(A[i]==NULL)
		{
            // deallocate all previously allocated memory
            for(int k=0; k<i; k++)
            {
            	free(A[k]);
			}
                
            free(A);
            
            return NULL; // allocation failed
        }
        
        for(int j=0; j<n; j++)
		{
            A[i][j] = malloc(R*sizeof(unsigned short));
            
            if(A[i][j]==NULL)
			{
                // deallocate all previously allocated memory
                for(int k=0; k<j; k++)
                {
                	free(A[i][k]);
				}
                    
                free(A[i]);
                
                for(int k=0; k<i; k++)
                {
                	for(int l=0; l<n; l++)
                    {
                        free(A[k][l]);
                    }
                    
                	free(A[k]);
				}
                    
                free(A);
                
                return NULL; // allocation failed
            }
        }
    }
    
    return A;
}

/**
 * This function deallocates memory for a 3D matrix of unsigned shorts.
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @param A A pointer to the first element of the 3D matrix to be deallocated.
 */
void free_gr_matrix(int m, int n, unsigned short*** A)
{
    for(int i=0; i<m; i++)
    {
        for(int j=0; j<n; j++)
        {
            free(A[i][j]);
        }
        free(A[i]);
    }
    free(A);
}

/**
 * This function sets all elements of a vector of unsigned shorts to zero.
 *
 * @param n The length of the vector.
 * @param A A pointer to the first element of the array.
 */
void zero_vector(int n, unsigned short* A)
{
	for(int i=0; i<n; i++)
	{
		A[i] = 0;
	}
}

/**
 * This function sets all elements of a 3D matrix of unsigned shorts to zero.
 * It zeros a group ring matrix where each element in the matrix hold an array of length R (the size of the group)
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @param A A pointer to the first element of the 3D matrix to be zeroed.
 */
void zero_gr_matrix(int m, int n, unsigned short*** A)
{
	for(int i=0; i<m; i++)
	{
		for(int j=0; j<n; j++)
		{
			for(int k=0; k<R; k++)
			{
				A[i][j][k] = 0;
			}
		}
	}
}

/**
 * This function generates a random number within a specified range [0, x).
 * It uses the 'randombytes' function, which employs AES-256 encryption for generating the random number.
 *
 * @param x The upper bound (exclusive) of the range within which the random number should be generated.
 * @return A random number in the range [0, x).
 */
unsigned short NIST_rng(int x)
{
    // Generate a random 32-bit number (4 bytes)
    uint32_t random_value;
    randombytes((unsigned char *)&random_value, sizeof(random_value));
    
    // Return a random number in the range [0, x)
    return random_value%x;
}

/**
 * This is a general function for computing the modulus of a number with respect to Q.
 * It ensures the result is non-negative by adding Q before taking the modulus.
 *
 * @param x The number to compute the modulus of.
 * @return The modulus of x with respect to Q.
 */
unsigned short mod_q(int x)
{
	return ((x%Q)+Q)%Q;
}

/**
 * This function computes the modulus of a number with respect to Q, assuming x is in the range [0, 2*Q-1).
 * It's designed for efficiency when adding two numbers that are already known to be in the range [0, Q).
 *
 * @param x The number to compute the modulus of.
 * @return The modulus of x with respect to Q.
 */
unsigned short a_mod_q(int x)
{
	if(x>=Q)
	{
		return x-Q;
	}
	else
	{
		return x;
	}
}

/**
 * This function computes the modulus of a number with respect to Q, assuming x is in the range (-Q, Q).
 * It's designed for efficiency when subtracting two numbers that are already known to be in the range [0, Q).
 *
 * @param x The number to compute the modulus of.
 * @return The modulus of x with respect to Q.
 */
unsigned short s_mod_q(int x)
{
	if(x<0)
	{
		return x+Q;
	}
	else
	{
		return x;
	}
}

/**
 * This function computes the modulus of a number with respect to Q. 
 * It's designed to be used when multiplying two numbers that are already known to be in the range [0, Q), or in cases where x is positive but potentially large.
 *
 * @param x The number to compute the modulus of.
 * @return The modulus of x with respect to Q.
 */
unsigned short p_mod_q(int x)
{
	return x%Q;
}

/**
 * This function calculates the modular multiplicative inverse of an integer 'a' modulo 'q'.
 * The modular multiplicative inverse of a number 'a' modulo 'q' is an integer 'b' such that:
 * (a*b) mod q = 1.
 * The function uses the Extended Euclidean Algorithm to find the modular multiplicative inverse.
 * If 'a' is not relatively prime to 'q', the modular inverse does not exist.
 *
 * @param a The integer to find the modular multiplicative inverse of. It should be in the range [0, q).
 * @param q The modulus.
 * @return The modular multiplicative inverse of 'a' modulo 'q'. If the modular inverse does not exist, the function returns 0.
 */
int multiplicative_inverse(int a, int q)
{
    int num = a; // This is the number for which we are finding the inverse.
    int denom = q; // This is the modulus.
    int y = 0; // This will be used to store intermediate values during the calculation.
    int b = 1; // This is the current best guess for the inverse.

    // This loop implements the Extended Euclidean Algorithm.
    while(num > 1)
    {
        int quotient = num/denom; // This is the integer part of num/denom.
        int remainder = denom; // This will hold the old value of denom for the swap operation below.

        // Perform the swap operation, similar to the Euclidean Algorithm.
        denom = num % denom;
        num = remainder;

        // Another swap operation is performed here, this time for the variables that will be used to calculate the inverse.
        remainder = y; 
        y = b - quotient * y; 
        b = remainder;
    }

    // Make sure b is positive. If it isn't, we add q to it to ensure it is within the correct range.
    if(b<0)
    {
        b += q;
    }

    // Return the inverse.
    return b;
}

/**
 * This function returns the sign of an integer.
 *
 * @param x The integer to check.
 * @return 1 if x is positive, -1 if x is negative, and 0 if x is zero.
 */
int signum(int x)
{
    if(x>0)
	{
        return 1;
    }
    else if(x<0)
	{
        return -1;
    }
    else
	{
        return 0;
    }
}

/**
 * This function performs the group operation for two elements of the alternating group A6, represented by their indices.
 * It first constructs the permutation resulting from the group operation, then computes the lexicographical index of 
 * that permutation and finally uses the lookup table to map this lexicographical index back to the index in A6.
 *
 * @param i1 The index of an element in A6
 * @param i2 The index of an element in A6
 * @return The index of the result of the group operation i1*i2 in A6
 */
int composition_index(int i1, int i2)
{
    // Permutation array to store the result of the composition
    unsigned char perm[P];

    // Calculate the composition of the two permutations
    for(int i=0; i<P; i++)
    {
        // The i-th element of the result is A6[i1][A6[i2][i]]
        // This means that we first apply the i2-th permutation (by looking up the i-th element),
        // then apply the i1-th permutation.
        perm[i] = A6[i1][A6[i2][i]];
    }
    
    // Initialize the index in the lexicographical order of permutations
    int lookup_index = 0;
    
    // Calculate the lexicographical index of the resulting permutation
    for(int i=0; i<P; i++)
    {
        // Count how many elements are smaller than perm[i] among the elements after it
        int smaller = 0;
        
        for(int j=i+1; j<P; j++)
        {
            if(perm[j]<perm[i])
            {
                smaller++;
            }
        }
        
        // The contribution of the i-th element to the lexicographical index is the number of smaller elements times the factorial of the number of remaining elements
        lookup_index += smaller*factorial[P-i-1];
    }

    // Use the lookup table to find the index of the resulting permutation in A6
    // Return the value at the computed index in the lookup array
    return lookup_A6[lookup_index];
}

/**
 * This function tries to solve an augmented matrix using Gauss-Jordan Elimination. 
 * @param n: The number of rows (and columns) in the matrix.
 * @param AM: The augmented matrix to be solved.
 * @return: True if the matrix can be solved, false otherwise (if the system of equations has no solution).
 */
bool solve_agumented_matrix(int n, unsigned short AM[n][n+1])
{
	for(int i=0; i<n; i++)
	{
		for(int k=i; k<n; k++)
		{
			if(AM[k][i]!=0)
			{
				if(k!=i)
				{
					for(int j=i; j<n+1; j++)
					{
						int t = AM[i][j];
						AM[i][j] = AM[k][j];
						AM[k][j] = t;
					}
				}
				
				break;
			}
		}
		
		if(AM[i][i]==0)
		{
			return 0; // solution does not exist
		}
		
		int u = multiplicative_inverse(AM[i][i], Q);
		
		for(int j=i; j<n+1; j++)
        {
            AM[i][j] = p_mod_q(u*AM[i][j]);
            
        }
		
		for(int j=0; j<n; j++)
		{
			if(AM[j][i]!=0 && j!=i)
			{
				int v = AM[j][i];
				
				for(int k=i; k<n+1; k++)
				{			
					AM[j][k] = s_mod_q(AM[j][k] - p_mod_q(v*AM[i][k]));
				}
			}
		}
	}
	
	return 1; // solution exists
}

/**
 * This function calculates the inverse of a vector in the group Gq.
 * @param vec: The vector to find the inverse of.
 * @param inverse_vec: The vector in which to store the inverse.
 * @return: True if the inverse exists, false otherwise.
 */
bool inverse_in_Gq(unsigned short* vec, unsigned short* inverse_vec)
{
	// Create an agumneted matrix and zero it
	unsigned short AM[R][R+1];
	for(int i=0; i<R; i++)
	{
		for(int j=0; j<R+1; j++)
		{
			AM[i][j] = 0;
		}
	}
	
	// The first element of the solution column is set to 1 and the rest remain zero (identity)
	AM[0][R] = 1;
	
	// We interatre through all elements of 'vec' and find its composition index with all possible indices and increment the value at the composition index by the value in 'vec'
	for(int j=0; j<R; j++)
	{
		if(vec[j]!=0)
		{
			for(int i=0; i<R; i++)
			{
				unsigned short index = composition_index(i,j);
				AM[index][i] = a_mod_q(AM[index][i] + vec[j]);
			}
		}
	}
	
	// Solves the agumented matrix
	bool condition = solve_agumented_matrix(R, AM);
	
	if(condition==1)
	{
		// The agumented matrix has a solution and hence we have an inverse
		for(int i=0; i<R; i++)
		{
			inverse_vec[i] = AM[i][R];
		}
	}
	
	// Could not solve the agumented matrix hence 'vec' does not have an inverse
	return condition;
}

/**
 * This function computes the product of two vectors in the group Gq.
 * @param vec1: The first vector.
 * @param vec2: The second vector.
 * @param result: The vector in which to store the product.
 * @param overwrite: If true, zeroes out the 'result' vector before computation.
 */
void product_in_Gq(unsigned short* vec1, unsigned short* vec2, unsigned short* result, bool overwrite)
{
	if(overwrite==1)
	{
		zero_vector(R, result);
	}
	
	for(int i=0; i<R; i++)
	{
		if(vec1[i]!=0)
		{
			for(int j=0; j<R; j++)
			{
				if(vec2[j]!=0)
				{
					unsigned short index = composition_index(i,j);
					result[index] = p_mod_q(result[index] + vec1[i]*vec2[j]);
				}
			}
		}
	}
}

/**
 * This function checks if a given group ring vector is empty (all elements are zero).
 * @param A: The group ring vector to check.
 * @return: True if the group ring vector is empty, false otherwise.
 */
bool is_gr_empty(unsigned short* A)
{
	for(int k=0; k<R; k++)
	{
		if(A[k]!=0)
		{
			return 0;
		}
	}
	
	return 1;
}

/**
 * This function multiplies two group ring matrices A and B, storing the result in group ring matrix C.
 * The operation performed is similar to standard matrix multiplication, but with additional
 * checks if elements are empty (all zero), and special multiplication operation for non-empty elements.
 *
 * @param m: The number of rows in the first matrix.
 * @param l: The number of columns in the first matrix (and rows in the second matrix).
 * @param n: The number of columns in the second matrix.
 * @param A: The first group ring matrix.
 * @param B: The second group ring matrix.
 * @param C: The group ring matrix in which to store the result.
 */
void grmatrix_multiply(int m, int l, int n, unsigned short*** A, unsigned short*** B, unsigned short*** C)
{
    // Initially set all elements of the result matrix C to zero.
    zero_gr_matrix(m, n, C);
    
    // We iterate over each cell of the result group ring matrix C
    for(int i=0; i<m; i++)
    {
        for(int j=0; j<n; j++)
        {
            // For each cell in C, we perform an operation similar to standard matrix multiplication,
            // where we multiply elements and sum them. However, here, instead of simple multiplication
            // and sum, we check if elements are empty and perform the product_in_Gq operation.
            for(int k=0; k<l; k++)
            {
                // We only perform the operation if both elements are non-empty
                if(is_gr_empty(A[i][k])==0 && is_gr_empty(B[k][j])==0)
                {
                    // The product of the non-empty elements from matrices A and B is added to the corresponding cell in matrix C
                    product_in_Gq(A[i][k], B[k][j], C[i][j], 0);
                }
            }
        }
    }
}

/**
 * This function computes a hash of a given message using SHAKE-256 and maps the result to a group ring matrix of integers modulo Q.
 * The SHAKE-256 is an extendable-output function (XOF) that belongs to the SHA-3 family of cryptographic hash functions.
 *
 * @param m A pointer to the message to be hashed.
 * @param mlen The length of the message.
 * @param h A pointer to the group ring matrix where the result will be stored.
 */
void hash_of_message(const unsigned char* m, unsigned long long mlen, unsigned short*** h)
{
	// Calculate the required length of the hash in bytes, 
	// ensuring that we have enough bytes to encode all coefficients of the group ring matrix.
	int required_length = ceil(M*R*log(Q)/log(256));
	
    // Allocate memory for the hash
    unsigned char required_hash[required_length];

    // Compute the SHAKE-256 hash of the message
    FIPS202_SHAKE256(m, mlen, required_hash, required_length);

    // Initialize the result group ring matrix to zeros
	zero_gr_matrix(M, 1, h);

	// Process each byte of the hash from the least significant to the most significant
    for(int j=required_length-1; j>=0; j--)
    {
    	// Initialize the carry to the current byte of the hash
        int carry = required_hash[j];
        
        // Process each coefficient of the group ring matrix from the least significant to the most significant
        for(int i=M-1; i>=0; i--)
        {
        	for(int k=R-1; k>=0; k--)
        	{
        		// Multiply the current coefficient by 256 and add the carry
        		int temp = 256*h[i][0][k] + carry;
        		
        		// Reduce the result modulo Q and store it in the group ring matrix
	            h[i][0][k] = temp%Q;
	            
	            // Update the carry
	            carry = temp/Q;
			}
		}
    }
}



