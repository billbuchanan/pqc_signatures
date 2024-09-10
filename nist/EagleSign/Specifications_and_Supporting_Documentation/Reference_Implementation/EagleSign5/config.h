/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef CONFIG_H
#define CONFIG_H

// #define USE_RDPMC
// #define EAGLESIGN_TIMING
//
#ifndef EAGLESIGN_MODE
#define EAGLESIGN_MODE 5
#endif

#if EAGLESIGN_MODE == 3
#define CRYPTO_ALGNAME "EagleSign3"
#define EAGLESIGN_NAMESPACETOP pq_eaglesign3_ref
#define EAGLESIGN_NAMESPACE(s) pq_eaglesign3_ref_##s
#elif EAGLESIGN_MODE == 5
#define CRYPTO_ALGNAME "EagleSign5"
#define EAGLESIGN_NAMESPACETOP pq_eaglesign5_ref
#define EAGLESIGN_NAMESPACE(s) pq_eaglesign5_ref_##s
#endif
#endif
