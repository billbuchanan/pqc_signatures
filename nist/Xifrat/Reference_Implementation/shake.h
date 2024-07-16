/* DannyNiu/NJF, 2018-02-08. Public Domain. */

#ifndef MySuiteA_shake_h
#define MySuiteA_shake_h 1

#include "mysuitea-common.h"
#include "sponge.h"

typedef struct shake_context {
    sponge_t    sponge;
    union {
        uint8_t     u8[200];
        uint64_t    u64[25];
    } state;
} shake_t, shake128_t, shake256_t;

void SHAKE128_Init(shake_t *restrict x);
void SHAKE256_Init(shake_t *restrict x);
void SHAKE_Write(shake_t *restrict x, const void *restrict data, size_t len);
void SHAKE_Final(shake_t *restrict x);
void SHAKE_Read(shake_t *restrict x, void *restrict data, size_t len);

#define cSHAKE(bits,q) (                                        \
        q==blockBytes ? (1600-bits*2)/8 :                       \
        q==contextBytes ? sizeof(struct shake_context) :        \
        q==InitFunc ? (IntPtr)SHAKE##bits##_Init :              \
        q==WriteFunc ? (IntPtr)SHAKE_Write :                    \
        q==XofFinalFunc ? (IntPtr)SHAKE_Final :                 \
        q==ReadFunc ? (IntPtr)SHAKE_Read :                      \
        0)
#define cSHAKE128(q) cSHAKE(128,q)
#define cSHAKE256(q) cSHAKE(256,q)

IntPtr iSHAKE128(int q);
IntPtr iSHAKE256(int q);

#endif /* MySuiteA_shake_h */
