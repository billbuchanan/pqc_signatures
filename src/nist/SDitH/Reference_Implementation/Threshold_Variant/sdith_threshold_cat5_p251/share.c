#include "share.h"

void compute_complete_share(mpc_share_t* share, const mpc_share_t* plain, const mpc_share_t* rnd[PARAM_NB_REVEALED], uint8_t eval) {
    vec_set(share, rnd[PARAM_NB_REVEALED-1], PARAM_SHARE_SIZE);
    if(eval != 0) {
        for(int i=PARAM_NB_REVEALED-2; i>=0; i--)
            vec_muladd2(share, eval, rnd[i], PARAM_SHARE_SIZE);
        vec_muladd2(share, eval, plain, PARAM_SHARE_SIZE);
    }
}

void compute_share_wit(mpc_wit_t* out, const mpc_share_t* plain, const mpc_share_t* rnd[PARAM_NB_REVEALED], uint8_t eval) {
    vec_set(out, rnd[PARAM_NB_REVEALED-1], PARAM_WIT_SIZE);
    if(eval != 0) {
        for(int i=PARAM_NB_REVEALED-2; i>=0; i--)
            vec_muladd2(out, eval, rnd[i], PARAM_WIT_SIZE);
        vec_muladd2(out, eval, plain, PARAM_WIT_SIZE);
    }
}

void compute_share_broadcast(mpc_broadcast_t* out, const mpc_broadcast_t* plain, const mpc_broadcast_t* rnd[PARAM_NB_REVEALED], uint8_t eval) {
    vec_set(out, rnd[PARAM_NB_REVEALED-1], PARAM_BR_SIZE);
    if(eval != 0) {
        for(int i=PARAM_NB_REVEALED-2; i>=0; i--)
            vec_muladd2(out, eval, rnd[i], PARAM_BR_SIZE);
        vec_muladd2(out, eval, plain, PARAM_BR_SIZE);
    }
}
