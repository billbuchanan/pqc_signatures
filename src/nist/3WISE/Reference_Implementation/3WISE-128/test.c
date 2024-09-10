#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


#include "api.h"

#define NUMBER_OF_KEYPAIRS 10     /* Number of keypairs that is generated during test */
#define SIGNATURES_PER_KEYPAIR 2 /* Number of times each keypair is used to sign a random document, and verify the signature */

double mean(double *arr, double len) {
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum / len;
}


int cmpfunc (const void * a, const void * b)
{
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
}


double median(double *arr, int len) {
    qsort(arr, len, sizeof(double), cmpfunc);
    
    if (len % 2 == 1) {
        return arr[len/ 2];
    }
    
    return (double)(arr[(len - 1) / 2] + arr[len / 2]) / 2;
}

/*
	Tests the execution of the keypair generation, signature generation and signature verification algorithms and prints timing results
*/

int main(void)
{
	int i, j, k;
	int message_size = 100;
	unsigned long long smlen;
	unsigned char m[message_size];
	unsigned char m2[message_size];
	unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
	unsigned char sm[message_size + CRYPTO_BYTES];
	clock_t cl;
    struct timeval start, end;
    long secs_used,micros_used;
    gettimeofday(&start, NULL);

	// Print key and signature sizes
	printf("Public Key takes %d B\n", CRYPTO_PUBLICKEYBYTES );
	printf("Secret Key takes %d B\n", CRYPTO_SECRETKEYBYTES );
	printf("Signature takes %d B\n\n", CRYPTO_BYTES );

	printf("Public Key takes %.2f kB\n", CRYPTO_PUBLICKEYBYTES / 1024.0);
	printf("Secret Key takes %.2f kB\n", CRYPTO_SECRETKEYBYTES / 1024.0);
	printf("Signature takes %.2f kB\n\n", CRYPTO_BYTES / 1024.0);

	srand((unsigned int) time(NULL));

    double gen_time_cpu[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR],
    sign_time_cpu[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR],
    verify_time_cpu[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR],
    gen_time_wall[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR],
    sign_time_wall[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR],
    verify_time_wall[NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR];

	for (i = 0; i < NUMBER_OF_KEYPAIRS ; i++) {

		// time key pair generation
		cl = clock();
        gettimeofday(&start, NULL);
		crypto_sign_keypair(pk, sk);
		cl = clock() - cl;
        gen_time_cpu[i] = ((double) cl)/(double)CLOCKS_PER_SEC * (double)1000;
        gettimeofday(&end, NULL);
        gen_time_wall[i] = ((((double)(end.tv_sec - start.tv_sec)*1000000) + end.tv_usec) - (start.tv_usec))/ (double)1000;

		for (j = 0; j < SIGNATURES_PER_KEYPAIR ; j++) {
			
			// pick a random message to sign
			for (k = 0; k < message_size; k++) {
				m[k] = ((unsigned char) rand());
			}

			// time signing algorithm
			cl = clock();
            gettimeofday(&start, NULL);
			crypto_sign(sm, &smlen, m, (unsigned long long) message_size, sk);
			cl = clock() - cl;
            sign_time_cpu[i*SIGNATURES_PER_KEYPAIR + j] = ((double)cl) / (double)CLOCKS_PER_SEC * 1000 ;
            gettimeofday(&end, NULL);
            sign_time_wall[i*SIGNATURES_PER_KEYPAIR + j] = ((((double)(end.tv_sec - start.tv_sec)*1000000) + end.tv_usec) - (start.tv_usec))/ (double)1000;

			printf("signed message length is %d B\n", smlen);

			// time verification algorithm
			cl = clock();
            gettimeofday(&start, NULL);
			if (crypto_sign_open(m2, &smlen, sm, smlen, pk) != 0) {
				printf("Verification of signature Failed!\n");
			}
			cl = clock() - cl;
            verify_time_cpu[i*SIGNATURES_PER_KEYPAIR + j] = ((double)cl) / (double)CLOCKS_PER_SEC * 1000;
            gettimeofday(&end, NULL);
            verify_time_wall[i*SIGNATURES_PER_KEYPAIR + j] = ((((double)(end.tv_sec - start.tv_sec)*1000000) + end.tv_usec) - (start.tv_usec))/ (double)1000; // nanosecs

			// check if recovered message length is correct
			if (smlen != message_size){
				printf("Wrong message size !\n");
			}
			// check if recovered message is correct
			for(k = 0 ; k<message_size ; k++){
				if(m[k]!=m2[k]){
					printf("Wrong message !\n");
					break;
				}
			}
            
            memset(m, 0, message_size);
            memset(sm, 0, message_size);
		}

	}

    printf("\nGeneration time cpu mean %fms\n", mean(gen_time_cpu, NUMBER_OF_KEYPAIRS));
    printf("Generation time cpu median %fms\n\n", median(gen_time_cpu, NUMBER_OF_KEYPAIRS));

    printf("Generation time wall mean %fms\n", mean(gen_time_wall, NUMBER_OF_KEYPAIRS));
    printf("Generation time wall median %fms\n\n", median(gen_time_wall, NUMBER_OF_KEYPAIRS));

    printf("Sign time cpu mean %fms\n", mean(sign_time_cpu, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));
    printf("Sign time cpu median %fms\n\n", median(sign_time_cpu, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));

    printf("Sign time wall mean %fms\n", mean(sign_time_wall, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));
    printf("Sign time wall median %fms\n\n", median(sign_time_wall, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));

    printf("Verify time cpu mean %fms\n", mean(verify_time_cpu, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));
    printf("Verify time cpu median %fms\n\n", median(verify_time_cpu, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));

    printf("Verify time wall mean %fms\n", mean(verify_time_wall, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));
    printf("Verify time wall median %fms\n\n", median(verify_time_wall, NUMBER_OF_KEYPAIRS*SIGNATURES_PER_KEYPAIR));

	return 0;
}
