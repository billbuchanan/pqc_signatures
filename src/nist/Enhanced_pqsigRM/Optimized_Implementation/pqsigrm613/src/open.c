#include "api.h"
#include "common.h"

long long cpucycles2(void) {
    unsigned long long result;
    __asm__ volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax" : "=a" (result) ::  "%rdx");
    return result;
}

void import_pk(const unsigned char *pk, matrix *H_pub){
    import_matrix(H_pub, pk);
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk){
    matrix *non_identity_hpub = new_matrix_with_pool(CODE_N - CODE_K - 1, CODE_K + 1, pk);
    matrix *syndrome_by_e     = new_matrix(1, CODE_N - CODE_K - 1);
    matrix* sign_tail = new_matrix(1, non_identity_hpub->ncols);

    uint64_t sign_i;
    uint64_t mlen_rx;
    unsigned char* m_rx;
    
    // import signed msg 
    // sign is (mlen, M, e, sign_i)
    mlen_rx = *(uint64_t*)sm;
    m_rx = (unsigned char*)malloc(mlen_rx);
    memcpy(m_rx, sm + sizeof(uint64_t), mlen_rx);
    matrix *sign = new_matrix_with_pool(1, CODE_N, sm + sizeof(uint64_t) + mlen_rx);

    sign_i = *(uint64_t*)(sm + sizeof(uint64_t) + mlen_rx + sign->nrows * sign->ncols/8);    

    if(hamming_weight(sign) > WEIGHT_PUB) {
        return VERIF_REJECT;
    }

    // syndrome by hash
    uint8_t randstr[non_identity_hpub->nrows/8 + 1];
    hash_message(randstr, m_rx, mlen_rx, sign_i);
    matrix *syndrome_by_hash = new_matrix_with_pool(1, non_identity_hpub->nrows, randstr);

    // syndrome by sign
    partial_replace(sign_tail, 0, 1, 0, sign_tail->ncols,
        sign, 0, non_identity_hpub->nrows);

    vec_mat_prod(syndrome_by_e, non_identity_hpub, sign_tail);
    // add sign_head
    vec_vec_add(syndrome_by_e, sign);

    if(!vec_vec_is_equal(syndrome_by_e, syndrome_by_hash)){
        return VERIF_REJECT;
    }

    memcpy(m, m_rx, mlen_rx);
    *mlen = mlen_rx;

    delete_matrix(sign);
    delete_matrix(non_identity_hpub);
    delete_matrix(sign_tail);
    delete_matrix(syndrome_by_hash);
    delete_matrix(syndrome_by_e);
    free(m_rx);

    return 0;
}
