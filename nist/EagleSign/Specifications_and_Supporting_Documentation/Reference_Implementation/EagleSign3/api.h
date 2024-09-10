#ifndef API_H
#define API_H

#include <stddef.h>
#include <stdint.h>

#define pq_eaglesign3_PUBLICKEYBYTES 1824
#define pq_eaglesign3_SECRETKEYBYTES 576
#define pq_eaglesign3_BYTES 2336

#define pq_eaglesign3_ref_PUBLICKEYBYTES pq_eaglesign3_PUBLICKEYBYTES
#define pq_eaglesign3_ref_SECRETKEYBYTES pq_eaglesign3_SECRETKEYBYTES
#define pq_eaglesign3_ref_BYTES pq_eaglesign3_BYTES

int pq_eaglesign3_ref_keypair(uint8_t *pk, uint8_t *sk);

int pq_eaglesign3_ref_signature(uint8_t *sig, size_t *siglen,
                                const uint8_t *m, size_t mlen,
                                const uint8_t *sk);

int pq_eaglesign3_ref(uint8_t *sm, size_t *smlen,
                      const uint8_t *m, size_t mlen,
                      const uint8_t *sk);

int pq_eaglesign3_ref_verify(const uint8_t *sig, size_t siglen,
                             const uint8_t *m, size_t mlen,
                             const uint8_t *pk);

int pq_eaglesign3_ref_open(uint8_t *m, size_t *mlen,
                           const uint8_t *sm, size_t smlen,
                           const uint8_t *pk);

#define pq_eaglesign3aes_ref_PUBLICKEYBYTES pq_eaglesign3_ref_PUBLICKEYBYTES
#define pq_eaglesign3aes_ref_SECRETKEYBYTES pq_eaglesign3_ref_SECRETKEYBYTES
#define pq_eaglesign3aes_ref_BYTES pq_eaglesign3_ref_BYTES

int pq_eaglesign3aes_ref_keypair(uint8_t *pk, uint8_t *sk);

int pq_eaglesign3aes_ref_signature(uint8_t *sig, size_t *siglen,
                                   const uint8_t *m, size_t mlen,
                                   const uint8_t *sk);

int pq_eaglesign3aes_ref(uint8_t *sm, size_t *smlen,
                         const uint8_t *m, size_t mlen,
                         const uint8_t *sk);

int pq_eaglesign3aes_ref_verify(const uint8_t *sig, size_t siglen,
                                const uint8_t *m, size_t mlen,
                                const uint8_t *pk);

int pq_eaglesign3aes_ref_open(uint8_t *m, size_t *mlen,
                              const uint8_t *sm, size_t smlen,
                              const uint8_t *pk);

#define pq_eaglesign5_PUBLICKEYBYTES 3616
#define pq_eaglesign5_SECRETKEYBYTES 1600
#define pq_eaglesign5_BYTES 3488

#define pq_eaglesign5_ref_PUBLICKEYBYTES pq_eaglesign5_PUBLICKEYBYTES
#define pq_eaglesign5_ref_SECRETKEYBYTES pq_eaglesign5_SECRETKEYBYTES
#define pq_eaglesign5_ref_BYTES pq_eaglesign5_BYTES

int pq_eaglesign5_ref_keypair(uint8_t *pk, uint8_t *sk);

int pq_eaglesign5_ref_signature(uint8_t *sig, size_t *siglen,
                                const uint8_t *m, size_t mlen,
                                const uint8_t *sk);

int pq_eaglesign5_ref(uint8_t *sm, size_t *smlen,
                      const uint8_t *m, size_t mlen,
                      const uint8_t *sk);

int pq_eaglesign5_ref_verify(const uint8_t *sig, size_t siglen,
                             const uint8_t *m, size_t mlen,
                             const uint8_t *pk);

int pq_eaglesign5_ref_open(uint8_t *m, size_t *mlen,
                           const uint8_t *sm, size_t smlen,
                           const uint8_t *pk);

#define pq_eaglesign5aes_ref_PUBLICKEYBYTES pq_eaglesign5_ref_PUBLICKEYBYTES
#define pq_eaglesign5aes_ref_SECRETKEYBYTES pq_eaglesign5_ref_SECRETKEYBYTES
#define pq_eaglesign5aes_ref_BYTES pq_eaglesign5_ref_BYTES

int pq_eaglesign5aes_ref_keypair(uint8_t *pk, uint8_t *sk);

int pq_eaglesign5aes_ref_signature(uint8_t *sig, size_t *siglen,
                                   const uint8_t *m, size_t mlen,
                                   const uint8_t *sk);

int pq_eaglesign5aes_ref(uint8_t *sm, size_t *smlen,
                         const uint8_t *m, size_t mlen,
                         const uint8_t *sk);

int pq_eaglesign5aes_ref_verify(const uint8_t *sig, size_t siglen,
                                const uint8_t *m, size_t mlen,
                                const uint8_t *pk);

int pq_eaglesign5aes_ref_open(uint8_t *m, size_t *mlen,
                              const uint8_t *sm, size_t smlen,
                              const uint8_t *pk);

#endif
