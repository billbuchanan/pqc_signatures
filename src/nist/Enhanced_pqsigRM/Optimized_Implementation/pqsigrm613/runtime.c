/*
NIST-developed software is provided by NIST as a public service. You may use, copy, and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify, and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
 
NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT, OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 
You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "src/rng.h"
#include "src/api.h"

#define	MAX_MARKER_LEN		50

#define RUN_SUCCESS          0
#define RUN_FILE_OPEN_ERROR -1
#define RUN_DATA_ERROR      -3
#define RUN_CRYPTO_FAILURE  -4

long long cpucycles(void);

char    AlgName[] = "Enhanced_pqsigRM613";

int
main()
{
    unsigned char       entropy_input[48];
    unsigned char       *m, *sm, *m1;
    uint64_t  mlen, smlen, mlen1;
    unsigned char       pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    int                 ret_val;

    for (int i=0; i<48; i++)
        entropy_input[i] = i;

    randombytes_init(entropy_input, NULL, 256);
    
    // Generate the public/private keypair
    long long tic, toc;
    for (int i = 0; i < 30; i++)
    {
        mlen = 33*(i+1);
        m = (unsigned char *)calloc(mlen, sizeof(unsigned char));
        m1 = (unsigned char *)calloc(mlen+CRYPTO_BYTES, sizeof(unsigned char));
        sm = (unsigned char *)calloc(mlen+CRYPTO_BYTES, sizeof(unsigned char));

        randombytes(m, mlen);

        tic = cpucycles();
        if ( (ret_val = crypto_sign_keypair(pk, sk)) != 0) {
            printf("crypto_sign_keypair returned <%d>\n", ret_val);
            return RUN_CRYPTO_FAILURE;
        }
        toc = cpucycles();
        printf("cycles keygen: %lld\n", toc-tic);
        
        tic = cpucycles();
        if ( (ret_val = crypto_sign(sm, (unsigned long long *)&smlen, m, mlen, sk)) != 0) {
            printf("crypto_sign returned <%d>\n", ret_val);
            return RUN_CRYPTO_FAILURE;
        }
        toc = cpucycles();
        printf("cycles sign: %lld\n", toc-tic);
        
        tic = cpucycles();
        if ( (ret_val = crypto_sign_open(m1, (unsigned long long *)&mlen1, sm, smlen, pk)) != 0) {
            printf("crypto_sign_open returned <%d>\n", ret_val);
            return RUN_CRYPTO_FAILURE;
        }
        toc = cpucycles();
        printf("cycles verif: %lld\n", toc-tic);
        
        if ( mlen != mlen1 ) {
            printf("crypto_sign_open returned bad 'mlen': Got <%lu>, expected <%lu>\n", mlen1, mlen);
            return RUN_CRYPTO_FAILURE;
        }
        
        if ( memcmp(m, m1, mlen) ) {
            printf("crypto_sign_open returned bad 'm' value\n");
            return RUN_CRYPTO_FAILURE;
        }
        
        free(m);
        free(m1);
        free(sm);
    }

    return RUN_SUCCESS;
}

long long cpucycles(void) {
    unsigned long long result;
    __asm__ volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax" : "=a" (result) ::  "%rdx");
    return result;
}
