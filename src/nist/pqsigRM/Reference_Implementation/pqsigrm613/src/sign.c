#include "api.h"
#include "common.h"
#include "nearest_vector.h"

int wgt(float *yc, float *yr)
{
    int i, w=0;
    for(i=0; i<CODE_N; i++)
        if(yc[i] != yr[i]) w++;
    return w;
}

void import_sk(const unsigned char *sk, uint16_t **Q, uint16_t **part_perm1, uint16_t **part_perm2, matrix* Hrep)
{
    *Q             = (uint16_t*)(sk);
    *part_perm1 = (uint16_t*)(sk+sizeof(uint16_t)*CODE_N);
    *part_perm2 = (uint16_t*)(sk+sizeof(uint16_t)*CODE_N + sizeof(uint16_t)*CODE_N/4);
    import_matrix(Hrep, sk+sizeof(uint16_t)*CODE_N + (sizeof(uint16_t)*CODE_N/4)*2);
}

// yr and yc are equal at the end.
void y_init(float *yc, float *yr, matrix* syndrome, uint16_t *Q){
    for(uint32_t i=0; i < syndrome->ncols; i++) {
        uint8_t bit = get_element(syndrome, 0, i);
        yc[i] = (bit == 0)? 1.:-1.;
    }
    for(uint32_t i = syndrome->ncols; i < CODE_N; i++) {
        yc[i] = 1.;
    }

    for(uint32_t i =0; i < CODE_N; i++) {
        yr[Q[i]] = yc[i];
    }
    for(uint32_t i =0; i < CODE_N; i++) {
        yc[i] = yr[i];
    }
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk){

    // read secret key(bit stream) into appropriate type.
    uint16_t *Q, *part_perm1, *part_perm2;
    matrix* Hrep = new_matrix(K_REP, (1<<RM_R));

    import_sk(sk, &Q, &part_perm1, &part_perm2, Hrep);
    
    // Do signing, decode until the a error vector wt <= w is achieved
    uint64_t sign_i;
    matrix *challenge= new_matrix(1, CODE_N - CODE_K - 1);

    float yc[CODE_N];
    float yr[CODE_N];
    float mempool[CODE_N];

    init_decoding(mempool);
    uint8_t randstr[challenge->ncols/8];

    while(1){
        // random number
        randombytes((unsigned char*)&sign_i, sizeof(uint64_t));
        hash_message(randstr, m, mlen, sign_i);
        randomize(challenge, randstr);
        
        // Find syndrome Sinv * challenge
        y_init(yc, yr, challenge, Q);
        
        // decode and find e
        // In the recursive decoding procedure,
        // Y is 1 when the received codeword is 0, o.w, -1
        recursive_decoding_mod(yc, RM_R, RM_M, 0, CODE_N, part_perm1, part_perm2, Hrep);
        
        // Check Hamming weight of e'
        if(wgt(yr, yc) <= WEIGHT_PUB) break;
    }

    // compute Qinv*e'
    matrix *sign = new_matrix(1, CODE_N);
    for(uint32_t i=0; i < CODE_N; i++){
        set_element(sign, 0, i, (uint8_t)(yr[Q[i]] != yc[Q[i]]));
    }

    // export message
    // sign is (mlen, M, e, sign_i)
    *(unsigned long long*)sm = mlen;
    memcpy(sm+sizeof(unsigned long long), m, mlen);
    export_matrix(sign, sm+sizeof(unsigned long long)+mlen);

    *(unsigned long long*)(sm + sizeof(unsigned long long) + mlen + sign->nrows * sign->ncols/8) 
        = sign_i;

    *smlen = sizeof(unsigned long long) + mlen + sign->nrows * sign->ncols/8 + sizeof(unsigned long long);
    
    delete_matrix(Hrep);
    delete_matrix(challenge);
    delete_matrix(sign);

    return 0;    
}
