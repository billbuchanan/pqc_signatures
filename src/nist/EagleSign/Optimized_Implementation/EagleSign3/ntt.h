/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef NTT_H
#define NTT_H

#include <stdint.h>
#include "params.h"

#define ntt EAGLESIGN_NAMESPACE(ntt)
void ntt(S_Q_SIZE a[N], unsigned logn);

#define invntt_tomont EAGLESIGN_NAMESPACE(invntt_tomont)
void invntt_tomont(S_Q_SIZE a[N], unsigned logn);

#define GMb EAGLESIGN_NAMESPACE(GMb)
extern const int16_t GMb[N];

#define iGMb EAGLESIGN_NAMESPACE(iGMb)
extern const int16_t iGMb[N];

#define basemul EAGLESIGN_NAMESPACE(basemul)
void basemul(int16_t r[2],
             const int16_t a[2],
             const int16_t b[2],
             int16_t zeta);

#endif
