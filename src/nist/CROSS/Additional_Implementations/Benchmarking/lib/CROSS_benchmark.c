/**
 *
 * Reference ISO-C11 Implementation of CROSS.
 *
 * @version 1.1 (March 2023)
 *
 * @author Alessandro Barenghi <alessandro.barenghi@polimi.it>
 * @author Gerardo Pelosi <gerardo.pelosi@polimi.it>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define HIGH_PERFORMANCE_X86_64
#include "timing_and_stat.h"
#include "CROSS.h"
#include "csprng_hash.h"


#define NUM_TESTS 10

void microbench(){
    welford_t timer;
    welford_init(&timer);

    uint64_t cycles;
    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        welford_update(&timer,(x86_64_rtdsc()-cycles)/1000.0);
    }
    fprintf(stderr,"microbench kCycles (avg,stddev):");
    welford_print(timer);
    printf("\n");

}

void info(){
    fprintf(stderr,"CROSS benchmarking utility\n");
    fprintf(stderr,"Code parameters: n= %d, k= %d, q=%d\n", N,K,Q);
    fprintf(stderr,"restriction size: z=%d\n",Z);
    fprintf(stderr,"Fixed weight challenge vector: %d rounds, weight %d \n",T,W);
    fprintf(stderr,"Private key: %luB\n", sizeof(prikey_t));
    fprintf(stderr,"Public key %luB\n", sizeof(pubkey_t));
    fprintf(stderr,"Signature: %luB\n", sizeof(sig_t));

}

void CROSS_sign_verify_speed(int print_tex){
    fprintf(stderr,"Computing number of clock cycles as the average of %d runs\n", NUM_TESTS);
    uint64_t cycles;
    pubkey_t pk;
    prikey_t sk;
    sig_t signature;
    char message[8] = "Signme!";

    welford_t timer_KG,timer_Sig,timer_Ver;
    welford_init(&timer_KG);
    welford_init(&timer_Sig);
    welford_init(&timer_Ver);

    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        CROSS_keygen(&sk,&pk);
        welford_update(&timer_KG,(x86_64_rtdsc()-cycles)/1000.0);
    }
    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        CROSS_sign(&sk,message,8,&signature);
        welford_update(&timer_Sig,(x86_64_rtdsc()-cycles)/1000.0);
    }
    int is_signature_still_ok = 1;
    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        int is_signature_ok = CROSS_verify(&pk,message,8,&signature);
        welford_update(&timer_Ver,(x86_64_rtdsc()-cycles)/1000.0);
        is_signature_still_ok = is_signature_ok || is_signature_still_ok;
    }
    if(print_tex){
      /* print a convenient machine extractable table row pair */
      printf("TIME & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(SHA2_HASH)
      printf("SHA-2 & ");
#elif defined(SHA3_HASH)
      printf("SHA-3 & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(SPEED)
      printf("Speed & ");
#endif
      // printf(" & ");
      welford_print_tex(timer_KG);
      printf(" & ");
      welford_print_tex(timer_Sig);
      printf(" & ");
      welford_print_tex(timer_Ver);
      printf("\n SPACE & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(SPEED)
      printf("Speed & ");
#endif
      printf(" %lu &", sizeof(prikey_t));
      printf(" %lu &", sizeof(pubkey_t));
      printf(" %lu ", sizeof(sig_t));
      printf(" \\\\\n");
    } else {
        info();
        printf("Timings (kcycles):\n");
        printf("Key generation kCycles (avg,stddev): ");
        welford_print(timer_KG);
        printf("\n");

        printf("Signature kCycles (avg,stddev): ");
        welford_print(timer_Sig);
        printf("\n");

        printf("Verification kCycles (avg,stddev):");
        welford_print(timer_Ver);
        printf("\n");
        fprintf(stderr,"Keygen-Sign-Verify: %s", is_signature_still_ok == 1 ? "functional\n": "not functional\n" );
    }
}

int iteration = 0;

int main(int argc, char* argv[]){
    initialize_csprng(&platform_csprng_state,
                      (const unsigned char *)"0123456789012345",16);
    fprintf(stderr,"CROSS reference implementation benchmarking tool\n");
    if ( (argc>1) &&
         (argv[1][0] == '-' ) &&
         (argv[1][1] == 'T' )){
        CROSS_sign_verify_speed(1);
    } else {
        CROSS_sign_verify_speed(0);
    }
    return 0;
}
