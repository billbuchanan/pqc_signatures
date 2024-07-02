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
#include "timing_and_stat.h"

#define NUM_TEST_ITERATIONS 1

#include "arith_unit_tests.h"
#include "CROSS.h"
#include "csprng_hash.h"


void info(void){
    fprintf(stderr,"CROSS functional testing utility\n");
#if defined(RSDP)
    fprintf(stderr,"RSDP Variant\n");
#elif defined(RSDPG)
    fprintf(stderr,"RSDPG Variant\n");
#endif
    fprintf(stderr,"Code parameters: n= %d, k= %d, q=%d\n", N,K,Q);
    fprintf(stderr,"restriction size: z=%d\n",Z);
    fprintf(stderr,"Fixed weight challenge vector: %d rounds, weight %d \n",T,W);
    fprintf(stderr,"Private key: %luB\n", sizeof(prikey_t));
    fprintf(stderr,"Public key %luB\n", sizeof(pubkey_t));
    fprintf(stderr,"Signature: %luB\n", sizeof(sig_t));
}

/* returns 1 if the test is successful, 0 otherwise */
int CROSS_sign_verify_test(){
    pubkey_t pk;
    prikey_t sk;
    sig_t signature;
    char message[8] = "Signme!";
    CROSS_keygen(&sk,&pk);
    CROSS_sign(&sk,message,8,&signature);
    int is_signature_ok;
    is_signature_ok = CROSS_verify(&pk,message,8,&signature);
    return is_signature_ok;
}

int main(int argc, char* argv[]){
    initialize_csprng(&platform_csprng_state,
                      (const unsigned char *)"012345678912345",
                      16);
    fprintf(stderr,"CROSS reference implementation functional testbench\n");
    info();
    int tests_ok = 0;
    for (int i = 0; i < NUM_TEST_ITERATIONS; i++) {
        int iteration_ok = 1;
        iteration_ok = iteration_ok && fq_arith_testing();
        fprintf(stderr,"Arith %d\n",iteration_ok);
        iteration_ok = iteration_ok && restr_belonging_test();
        fprintf(stderr,"Restr %d\n",iteration_ok);
        iteration_ok = iteration_ok && signature_invariant_test();
        fprintf(stderr,"Sig_invariant %d\n",iteration_ok);
        iteration_ok = iteration_ok && CROSS_sign_verify_test();
        fprintf(stderr,"Full %d\n",iteration_ok);
        tests_ok += iteration_ok;
    }
    fprintf(stderr,"\n%d tests functional out of %d\n",tests_ok,NUM_TEST_ITERATIONS);
    return 0;
}
