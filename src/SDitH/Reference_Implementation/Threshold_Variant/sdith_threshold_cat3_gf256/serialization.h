#ifndef SDITH_SERIALIZATION_H
#define SDITH_SERIALIZATION_H

#include "parameters-all.h"
#include "mpc-all.h"

// Witness
#ifndef PARAM_WIT_SHORT_SIZE
static inline void wit_serialize(uint8_t* buf, const mpc_wit_t* wit) {
    vec_set(buf, wit, PARAM_WIT_SIZE);
}
static inline void wit_deserialize(mpc_wit_t const** wit, const uint8_t* buf, const uint8_t* memory) {
    (void) memory; // No need to use additional memory
    (*wit) = (const mpc_wit_t*) buf;
}
#define PARAM_WIT_SHORT_SIZE PARAM_WIT_SIZE
#define PARAM_WIT_PARSED_SIZE (0)
#else
#define wit_serialize(buf, wit) wit_compress(buf, wit)
static inline void wit_deserialize(mpc_wit_t const** wit, const uint8_t* buf, uint8_t* memory) {
    wit_decompress((mpc_wit_t*) memory, buf);
    (*wit) = (const mpc_wit_t*) memory;
}
#define PARAM_WIT_PARSED_SIZE PARAM_WIT_SIZE
#endif

// Unif
#ifndef PARAM_UNIF_SHORT_SIZE
static inline void unif_serialize(uint8_t* buf, const mpc_unif_t* unif) {
    vec_set(buf, unif, PARAM_UNIF_SIZE);
}
static inline void unif_deserialize(mpc_unif_t const** unif, const uint8_t* buf, const uint8_t* memory) {
    (void) memory; // No need to use additional memory
    (*unif) = (const mpc_unif_t*) buf;
}
#define PARAM_UNIF_SHORT_SIZE PARAM_UNIF_SIZE
#define PARAM_UNIF_PARSED_SIZE (0)
#else
#define unif_serialize(buf, unif) unif_compress(buf, unif)
static inline void unif_deserialize(mpc_unif_t const** unif, const uint8_t* buf, uint8_t* memory) {
    unif_decompress((mpc_unif_t*) memory, buf);
    (*unif) = (const mpc_unif_t*) memory;
}
#define PARAM_UNIF_PARSED_SIZE PARAM_UNIF_SIZE
#endif

// Corr
#ifndef PARAM_CORR_SHORT_SIZE
static inline void corr_serialize(uint8_t* buf, const mpc_corr_t* corr) {
    vec_set(buf, corr, PARAM_CORR_SIZE);
}
static inline void corr_deserialize(mpc_corr_t const** corr, const uint8_t* buf, const uint8_t* memory) {
    (void) memory; // No need to use additional memory
    (*corr) = (const mpc_corr_t*) buf;
}
#define PARAM_CORR_SHORT_SIZE PARAM_CORR_SIZE
#define PARAM_CORR_PARSED_SIZE (0)
#else
#define corr_serialize(buf, corr) corr_compress(buf, corr)
static inline void corr_deserialize(mpc_corr_t const** corr, const uint8_t* buf, uint8_t* memory) {
    corr_decompress((mpc_corr_t*) memory, buf);
    (*corr) = (const mpc_corr_t*) memory;
}
#define PARAM_CORR_PARSED_SIZE PARAM_CORR_SIZE
#endif

// Broadcast
#ifndef PARAM_BR_SHORT_SIZE
static inline void br_serialize(uint8_t* buf, const mpc_broadcast_t* br) {
    vec_set(buf, br, PARAM_BR_SIZE);
}
static inline void br_deserialize(mpc_broadcast_t const** br, const uint8_t* buf, const uint8_t* memory) {
    (void) memory; // No need to use additional memory
    (*br) = (const mpc_broadcast_t*) buf;
}
#define PARAM_BR_SHORT_SIZE PARAM_BR_SIZE
#define PARAM_BR_PARSED_SIZE (0)
#else
#define br_serialize(buf, br) br_compress(buf, br)
static inline void br_deserialize(mpc_broadcast_t const** br, const uint8_t* buf, uint8_t* memory) {
    br_decompress((mpc_broadcast_t*) memory, buf);
    (*br) = (const mpc_broadcast_t*) memory;
}
#define PARAM_BR_PARSED_SIZE PARAM_BR_SIZE
#endif

#endif /* SDITH_SERIALIZATION_H */
