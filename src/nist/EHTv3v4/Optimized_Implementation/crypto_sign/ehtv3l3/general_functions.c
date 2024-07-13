#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "rng.h"
#include "parameters.h"
#include "keccak.h"


/**
 * This function generates a random number within a specified range [0, x).
 * It uses the 'randombytes' function, which employs AES-256 encryption for generating the random number.
 *
 * @param x The upper bound (exclusive) of the range within which the random number should be generated.
 * @return A random number in the range [0, x).
 */
int NIST_rng(int x)
{
    // Generate a random 32-bit number (4 bytes)
    uint32_t random_value;
    randombytes((unsigned char *)&random_value, sizeof(random_value));
    
    // Return a random number in the range [0, x)
    return random_value%x;
}

/**
 * This function allocates memory for an array of unsigned chars.
 * 
 * @param n The length of the array to be allocated.
 * @return A pointer to the first element of the newly allocated array.
 *         If the allocation fails, NULL is returned.
 */
unsigned char* allocate_unsigned_char_vector_memory(int n)
{
    unsigned char *A = malloc(n*sizeof(unsigned char));
    return A;
}

/**
 * This function allocates memory for a 2D matrix of unsigned chars.
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @return A pointer to the first element of the newly allocated matrix.
 *         If the allocation fails at any point, any memory that was 
 *         successfully allocated is freed and NULL is returned.
 */
unsigned char** allocate_unsigned_char_matrix_memory(int m, int n)
{
    unsigned char** A = malloc(m*sizeof(unsigned char*));
    
    if(A==NULL)
    {
        return NULL; // Allocation failed
    }
    
    for(int i=0; i<m; i++)
    {
        A[i] = malloc(n*sizeof(unsigned char));
        
        if(A[i]==NULL)
        {
            // Deallocate all previously allocated memory
            for(int k=0; k<i; k++)
            {
                free(A[k]);
            }
            
            free(A);
            
            return NULL; // Allocation failed
        }
    }
    
    return A;
}

/**
 * This function allocates memory for a 2D matrix of unsigned shorts.
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @return A pointer to the first element of the newly allocated matrix.
 *         If the allocation fails at any point, any memory that was 
 *         successfully allocated is freed and NULL is returned.
 */
unsigned short** allocate_unsigned_short_matrix_memory(int m, int n)
{
    unsigned short** A = malloc(m*sizeof(unsigned short*));
    
    if(A==NULL)
    {
        return NULL; // Allocation failed
    }
    
    for(int i=0; i<m; i++)
    {
        A[i] = malloc(n*sizeof(unsigned short));
        
        if(A[i]==NULL)
        {
            // Deallocate all previously allocated memory
            for(int k=0; k<i; k++)
            {
                free(A[k]);
            }
            
            free(A);
            
            return NULL; // Allocation failed
        }
    }
    
    return A;
}

/**
 * This function frees the memory allocated for a 2D matrix of unsigned chars.
 *
 * @param m The number of rows in the matrix.
 * @param A A pointer to the first element of the matrix to be freed.
 */
void free_matrix(int m, unsigned char** A)
{
    for(int i=0; i<m; i++)
    {
        free(A[i]); // Free each row of the matrix
    }
    free(A); // Free the array of pointers
}

/**
 * This function frees the memory allocated for a 2D matrix of unsigned shorts.
 *
 * @param m The number of rows in the matrix.
 * @param A A pointer to the first element of the matrix to be freed.
 */
void free_short_matrix(int m, unsigned short** A)
{
    for(int i=0; i<m; i++)
    {
        free(A[i]); // Free each row of the matrix
    }
    free(A); // Free the array of pointers
}

/**
 * This function sets all elements of an array of unsigned chars to zero.
 *
 * @param n The length of the array.
 * @param A A pointer to the first element of the array.
 */
void zero_vector(int n, unsigned char* A)
{
    for(int i=0; i<n; i++)
    {
        A[i] = 0; // Set each element of the array to zero
    }
}

/**
 * This function sets all elements of a 2D matrix of unsigned chars to zero.
 *
 * @param m The number of rows in the matrix.
 * @param n The number of columns in the matrix.
 * @param A A pointer to the first element of the matrix.
 */
void zero_matrix(int m, int n, unsigned char** A)
{
    for(int i=0; i<m; i++)
    {
        for(int j=0; j<n; j++)
        {
            A[i][j] = 0; // Set each element of the matrix to zero
        }
    }
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
 * This is a general function for computing the modulus of a number with respect to Q.
 * It ensures the result is non-negative by adding Q before taking the modulus.
 *
 * @param x The number to compute the modulus of.
 * @return The modulus of x with respect to Q.
 */
int mod_q(int x)
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
int a_mod_q(int x)
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
int s_mod_q(int x)
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
int p_mod_q(int x)
{
	return x%Q;
}

/**
 * This function converts a decimal integer to a binary number represented as a boolean vector.
 *
 * @param decimal The decimal integer to be converted.
 * @param bin_vec A pointer to the boolean vector where the binary representation will be stored.
 * @param num_bits The number of bits in the binary representation. This must be equal to or greater than the number of bits required to represent the decimal number.
 */
void decimal_to_binary(int decimal, bool *bin_vec, int num_bits)
{
    // Initialize the binary vector to zeros
	for(int i=0; i<num_bits; i++)
	{
		bin_vec[i] = 0;
	}

    // Convert the decimal number to binary and store it in the binary vector
	for(int i=num_bits-1; i>=0; i--)
	{
        bin_vec[i] = decimal & 1;  // Get the least significant bit of the decimal number
        decimal >>= 1;  // Shift the decimal number to the right by 1 bit (i.e., divide it by 2)
    }
}

/**
 * This function multiplies two matrices A and B, and stores the result in matrix C.
 * The function also performs modular reduction with respect to Q on each element of the resulting matrix.
 *
 * @param m The number of rows in matrix A.
 * @param l The number of columns in matrix A and the number of rows in matrix B.
 * @param n The number of columns in matrix B.
 * @param A A pointer to the first matrix.
 * @param B A pointer to the second matrix.
 * @param C A pointer to the matrix where the result will be stored.
 */
void matrix_multiply(int m, int l, int n, unsigned char** A, unsigned char **B, unsigned char **C)
{
    // Declare and initialize the transpose of matrix B
	unsigned char Bt[n][l];
	for(int i=0; i<l; ++i)
	{
		for(int j=0; j<n; ++j)
		{
			Bt[j][i] = B[i][j];  // Transpose matrix B
		}
	}

    // Perform matrix multiplication
	for(int i=0; i<m; i++)
	{
		for(int j=0; j<n; j++) 
		{
			int sum = 0;
			
            // Calculate the dot product of the i-th row of matrix A and the j-th row of the transpose of matrix B
			for(int k=0; k<l; k++) 
			{
				sum = sum + A[i][k]*Bt[j][k];
			}

            // Store the result in matrix C, after performing modular reduction with respect to Q
			C[i][j] = p_mod_q(sum);
		}
	}
}

/**
 * This function computes a hash of a given message using SHAKE-256 and maps the result to a matrix of integers modulo Q.
 * The SHAKE-256 is an extendable-output function (XOF) that belongs to the SHA-3 family of cryptographic hash functions.
 *
 * @param m A pointer to the message to be hashed.
 * @param mlen The length of the message.
 * @param h A pointer to the matrix where the result will be stored.
 */
void hash_of_message(const unsigned char* m, unsigned long long mlen, unsigned char** h)
{
    // Calculate the required length of the hash in bytes, 
	// ensuring that we have enough bytes to encode all coefficients of the matrix.
	int required_length = ceil(M*log(Q)/log(256));
	
    // Allocate memory for the hash
    unsigned char required_hash[required_length];

    // Compute the SHAKE-256 hash of the message
    FIPS202_SHAKE256(m, mlen, required_hash, required_length);

    // Initialize the result matrix to zeros
	zero_matrix(M, 1, h);

    // Process each byte of the hash from the least significant to the most significant
    for(int j=required_length-1; j>=0; j--)
    {
        // Initialize the carry to the current byte of the hash
        int carry = required_hash[j];
        
        // Process each coefficient of the matrix from the least significant to the most significant
        for(int i=M-1; i>=0; i--)
        {
        	// Multiply the current coefficient by 256 and add the carry
            int temp = 256*h[i][0] + carry;
            
            // Reduce the result modulo Q and store it in the matrix
            h[i][0] = temp%Q;
            
            // Update the carry
            carry = temp/Q;
		}
    }
}


