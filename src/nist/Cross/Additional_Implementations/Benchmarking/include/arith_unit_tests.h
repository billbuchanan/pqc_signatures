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

#include "fq_arith.h"
#include "csprng_hash.h"
#include <stdio.h>


static inline
int trivial_is_restricted(FZ_ELEM* v, int len){
    int is_ok = 1;
    for(int i =0; i<len; i++){
        is_ok = is_ok ||
                ( (v[i] == 1) || (v[i] == 2) || (v[i] == 4) || (v[i] == 8) ||
                  (v[i] == 16) || (v[i] == 32) || (v[i] == 64));
    }
    return is_ok;
}

int restr_belonging_test(void){
    uint8_t seed[SEED_LENGTH_BYTES];
    randombytes(seed,SEED_LENGTH_BYTES);
    CSPRNG_STATE_T CSPRNG_state;
    initialize_csprng(&CSPRNG_state,seed,SEED_LENGTH_BYTES);

    FZ_ELEM a[N]={0};
    CSPRNG_zz_vec(a,&CSPRNG_state);

    return trivial_is_restricted(a,N) == is_zz_vec_in_restr_group(a);
}

int restr_by_matr_test(void){
    uint8_t seed[SEED_LENGTH_BYTES];
    randombytes(seed,SEED_LENGTH_BYTES);
    CSPRNG_STATE_T CSPRNG_state;
    initialize_csprng(&CSPRNG_state,seed,SEED_LENGTH_BYTES);

    FZ_ELEM e[N],e_tilde[N];
    CSPRNG_zz_vec(e, &CSPRNG_state);

    FQ_ELEM V_tr[N-K][K];
    CSPRNG_fq_mat(V_tr,&CSPRNG_state);

    FQ_ELEM syn_1[N-K] = {0},
            syn_2[N-K] = {0},
            delta[N-K];

    for(int i=0;i<N;i++){
        e_tilde[i] = (e[i]+2) %7;
    }
    restr_vec_by_fq_matrix(syn_1,e,V_tr);
    restr_vec_by_fq_matrix(syn_2,e_tilde,V_tr);
    fq_dz_norm_synd(syn_1);
    fq_dz_norm_synd(syn_2);
    for(int i=0;i<N-K;i++){
        delta[i] = ((Q+syn_2[i])- (3*syn_1[i]%Q)) % Q;
    }
    int outcome = (memcmp(delta,syn_1,N-K) == 0);
    if(!outcome){
        fprintf(stderr,"delta: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", delta[i]);
        }
        fprintf(stderr,"]\n");
        fprintf(stderr,"syn_1: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", syn_1[i]);
        }
        fprintf(stderr,"]\n");

    }
    return outcome;
}


void prettyprint(char* name, FQ_ELEM *v, int l){
    fprintf(stderr,"%s: [",name);
    for(int i=0; i<l; i++){
        fprintf(stderr," %d, ", v[i]);
    }
    fprintf(stderr,"]\n");
}
#if defined(RSDP)
int signature_invariant_test(void){
    uint8_t seed[SEED_LENGTH_BYTES];
    randombytes(seed,SEED_LENGTH_BYTES);
    CSPRNG_STATE_T CSPRNG_state;
    initialize_csprng(&CSPRNG_state,seed,SEED_LENGTH_BYTES);

    /* keygen */
    FZ_ELEM e[N];
    FQ_ELEM V_tr[N-K][K];
    CSPRNG_zz_vec(e, &CSPRNG_state);
    CSPRNG_fq_mat(V_tr,&CSPRNG_state);
    FQ_ELEM public_key_synd[N-K] = {0};
    restr_vec_by_fq_matrix(public_key_synd,e,V_tr);
    fq_dz_norm_synd(public_key_synd);


    FZ_ELEM e_tilde[N],sigma[N];

    CSPRNG_zz_vec(e_tilde, &CSPRNG_state);

    restr_vec_sub(sigma,e,e_tilde);
    FQ_ELEM tau[N];
    convert_restr_vec_to_fq(tau,sigma);

    FQ_ELEM u[N],u_tilde[N];
    CSPRNG_fq_vec(u_tilde, &CSPRNG_state);

    fq_vec_by_fq_vec_pointwise(u,tau,u_tilde);

    FQ_ELEM syn_tilde_sig[N-K] = {0},
            syn_tilde_ver[N-K] = {0};

    fq_vec_by_fq_matrix(syn_tilde_sig,u,V_tr);
    fq_dz_norm_synd(syn_tilde_sig);

    FQ_ELEM y_tilde[N], y_sig[N];
    FQ_ELEM beta = 1;
    /* y = u_tilde + beta * e*/
    fq_vec_by_restr_vec_scaled(y_sig, e_tilde, beta, u_tilde);
    fq_dz_norm(y_sig);

    /* Verify starts here */
    fq_vec_by_fq_vec_pointwise(y_tilde,tau,y_sig);

    FQ_ELEM syn_tilde_tmp_ver[N-K] = {0};
    fq_vec_by_fq_matrix(syn_tilde_tmp_ver,y_tilde,V_tr);
    fq_dz_norm_synd(syn_tilde_tmp_ver);

    fq_synd_minus_fq_vec_scaled(syn_tilde_ver,
                                syn_tilde_tmp_ver,
                                beta,
                                public_key_synd);
    fq_dz_norm_synd(syn_tilde_ver);

    int outcome = (memcmp(syn_tilde_ver,syn_tilde_sig,N-K) == 0);
    if(!outcome){
        fprintf(stderr,"syn_tilde_sig: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", syn_tilde_sig[i]);
        }
        fprintf(stderr,"]\n");
        fprintf(stderr,"syn_tilde_ver: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", syn_tilde_ver[i]);
        }
        fprintf(stderr,"]\n");

    }
    return outcome;
}
#elif defined(RSDPG)
static
void expand_public_seed(FQ_ELEM V_tr[N-K][K],
                        FZ_ELEM W_tr[N-M][M],
                        const uint8_t seed_pub[SEED_LENGTH_BYTES]){
  uint8_t seedV_seed_W[2][SEED_LENGTH_BYTES];
  CSPRNG_STATE_T CSPRNG_state_mat;
  initialize_csprng(&CSPRNG_state_mat, seed_pub, SEED_LENGTH_BYTES);
  csprng_randombytes((uint8_t *)seedV_seed_W,
                     2*SEED_LENGTH_BYTES,
                     &CSPRNG_state_mat);

  initialize_csprng(&CSPRNG_state_mat, seedV_seed_W[0], SEED_LENGTH_BYTES);
  CSPRNG_fq_mat(V_tr,&CSPRNG_state_mat);

  initialize_csprng(&CSPRNG_state_mat, seedV_seed_W[1], SEED_LENGTH_BYTES);
  CSPRNG_fz_mat(W_tr,&CSPRNG_state_mat);
}

static
void expand_private_seed(FZ_ELEM eta[N],
                         FZ_ELEM zeta[M],
                         FQ_ELEM V_tr[N-K][K],
                         FZ_ELEM W_tr[N-M][M],
                         const uint8_t seed[SEED_LENGTH_BYTES]){
  uint8_t seede_seed_pub[2][SEED_LENGTH_BYTES];
  CSPRNG_STATE_T CSPRNG_state;
  initialize_csprng(&CSPRNG_state,seed,SEED_LENGTH_BYTES);
  csprng_randombytes((uint8_t *)seede_seed_pub,
                     2*SEED_LENGTH_BYTES,
                     &CSPRNG_state);

  expand_public_seed(V_tr,W_tr,seede_seed_pub[1]);
  CSPRNG_STATE_T CSPRNG_state_eta;
  initialize_csprng(&CSPRNG_state_eta, seede_seed_pub[0], SEED_LENGTH_BYTES);
  CSPRNG_zz_inf_w(zeta,&CSPRNG_state_eta);
  fz_inf_w_by_fz_matrix(eta,zeta,W_tr);
  fz_dz_norm_sigma(eta);
}
int signature_invariant_test(void){
    uint8_t seed[SEED_LENGTH_BYTES];
    randombytes(seed,SEED_LENGTH_BYTES);
    CSPRNG_STATE_T CSPRNG_state;
    initialize_csprng(&CSPRNG_state,seed,SEED_LENGTH_BYTES);

    FZ_ELEM eta[N];
    FZ_ELEM zeta[M];
    FQ_ELEM V_tr[N-K][K];
    FZ_ELEM W_tr[N-M][M];
    expand_private_seed(eta, zeta, V_tr, W_tr,seed);

    /* keygen */
    FQ_ELEM public_key_synd[N-K] = {0};
    restr_vec_by_fq_matrix(public_key_synd,eta,V_tr);
    fq_dz_norm_synd(public_key_synd);

    /* sign */
    FZ_ELEM zeta_tilde[M],eta_tilde[N],delta[M],sigma[N];

    // CSPRNG_zz_vec(e_tilde, &CSPRNG_state);
    CSPRNG_zz_inf_w(zeta_tilde, &CSPRNG_state);
    restr_inf_w_sub(delta, zeta,zeta_tilde);

    fz_inf_w_by_fz_matrix(eta_tilde,zeta_tilde,W_tr);

    restr_vec_sub(sigma,eta,eta_tilde);

    FQ_ELEM tau[N];
    convert_restr_vec_to_fq(tau,sigma);

    FQ_ELEM u[N],u_tilde[N];
    CSPRNG_fq_vec(u_tilde, &CSPRNG_state);

    fq_vec_by_fq_vec_pointwise(u,tau,u_tilde);

    FQ_ELEM syn_tilde_sig[N-K] = {0},
            syn_tilde_ver[N-K] = {0};

    fq_vec_by_fq_matrix(syn_tilde_sig,u,V_tr);
    fq_dz_norm_synd(syn_tilde_sig);

    FQ_ELEM y_tilde[N], y_sig[N];
    FQ_ELEM beta = 1;
    /* y = u_tilde + beta * e*/
    fq_vec_by_restr_vec_scaled(y_sig, eta_tilde, beta, u_tilde);
    fq_dz_norm(y_sig);

    /* Verify starts here */
    FZ_ELEM sigma_ver[N];
    FQ_ELEM tau_ver[N];
    fz_inf_w_by_fz_matrix(sigma_ver,delta,W_tr);
    convert_restr_vec_to_fq(tau_ver,sigma);

    fq_vec_by_fq_vec_pointwise(y_tilde,tau_ver,y_sig);

    FQ_ELEM syn_tilde_tmp_ver[N-K] = {0};
    fq_vec_by_fq_matrix(syn_tilde_tmp_ver,y_tilde,V_tr);
    fq_dz_norm_synd(syn_tilde_tmp_ver);

    fq_synd_minus_fq_vec_scaled(syn_tilde_ver,
                                syn_tilde_tmp_ver,
                                beta,
                                public_key_synd);
    fq_dz_norm_synd(syn_tilde_ver);

    int outcome = (memcmp(syn_tilde_ver,syn_tilde_sig,N-K) == 0);
    if(!outcome){
        fprintf(stderr,"syn_tilde_sig: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", syn_tilde_sig[i]);
        }
        fprintf(stderr,"]\n");
        fprintf(stderr,"syn_tilde_ver: [");
        for(int i=0;i<N-K;i++){
            fprintf(stderr," %d, ", syn_tilde_ver[i]);
        }
        fprintf(stderr,"]\n");

    }
    return outcome;
}
#endif



int fq_arith_testing(void){
  /* test restr_to_val against junior grade school exponentiation */
  for (FZ_ELEM exp = 0; exp < Z; exp++){
    uint32_t check = 1;
    for (int i = exp; i>0; i--){
        check = (check*RESTR_G_GEN) % Q;
    }

    FQ_ELEM fun;
    fun = RESTR_TO_VAL(exp);
    if( fun != check){
            fprintf(stderr,"mismatch: 16**%u = %u != %u\n",exp,check,fun);
    }
  }
  return 1;
}
