#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "api.h"
#include "utils.h"

int randombytes(unsigned char* x, unsigned long long xlen) {
    for(unsigned long long j=0; j<xlen; j++)
        x[j] = rand();
}

int main(void) {
    srand((unsigned int) time(NULL));

    int nb_tests = 1;
    print_configuration();

    int ret;

    // Execution
    int score = 0;
    for(int i=0; i<nb_tests; i++) {
        // Generate the keys
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];

        // Select the message
        #define MLEN 32
        uint8_t m[MLEN] = {1, 2, 3, 4};
        uint8_t m2[MLEN] = {0};
        unsigned long long m2len;
        
        // Sign the message
        uint8_t sm[MLEN+CRYPTO_BYTES];
        unsigned long long smlen;

        // Write keys and signature
        FILE *fptr;
        fptr = fopen("bench-sig.txt","r");
        if(fptr == NULL) {
            printf("Failure (num %d): failed to open file\n", i);
            continue;
        }
        for(unsigned int j=0; j<CRYPTO_PUBLICKEYBYTES; j++)
            fscanf(fptr, "%hhu ", &pk[j]);
        fscanf(fptr, "\n");
        for(unsigned int j=0; j<CRYPTO_SECRETKEYBYTES; j++)
            fscanf(fptr, "%hhu ", &sk[j]);
        fscanf(fptr, "\n");
        smlen = 0;
        int val = 1;
        while(val > 0) {
            val = fscanf(fptr, "%hhu ", &sm[smlen]);
            smlen++;
        }
        smlen--;
        fclose(fptr);

        // Verify/Open the signature
        ret = crypto_sign_open(m2, &m2len, sm, smlen, pk);
        if(ret) {
            printf("Failure (num %d): crypto_sign_open\n", i);
            continue;
        }
        
        // Test of correction of the primitives
        if(m2len != MLEN) {
            printf("Failure (num %d): message size does not match\n", i);
            continue;
        }
        for(int h=0; h<MLEN; h++)
            if(m[h] != m2[h]) {
                printf("Failure (num %d): message does not match (char %d)\n", i, h);
                continue;
            }

        score++;
    }

    // Display Infos
    printf("===== SUMMARY =====\n");
    printf("Correctness: %d/%d\n", score, nb_tests);
    printf("Read from bench-sig.txt\n");

    return 0;
}
