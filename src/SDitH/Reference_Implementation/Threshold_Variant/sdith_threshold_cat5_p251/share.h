#ifndef SDITH_SHARE_H
#define SDITH_SHARE_H

#include "parameters-all.h"
#include "mpc-all.h"

void compute_complete_share(mpc_share_t* share, const mpc_share_t* plain, const mpc_share_t* rnd[PARAM_NB_REVEALED], uint8_t eval);
void compute_share_wit(mpc_wit_t* out, const mpc_share_t* plain, const mpc_share_t* rnd[PARAM_NB_REVEALED], uint8_t eval);
void compute_share_broadcast(mpc_broadcast_t* out, const mpc_broadcast_t* plain, const mpc_broadcast_t* rnd[PARAM_NB_REVEALED], uint8_t eval);

#endif /* SDITH_SHARE_H */
