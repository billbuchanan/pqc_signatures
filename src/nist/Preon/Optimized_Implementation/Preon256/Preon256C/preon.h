#ifndef _PREON_H__
#define _PREON_H__

#include <stddef.h>
#include <stdint.h>

size_t publickey_len(int aes_size);
size_t privatekey_len(int aes_size);
void keygen(int aes_size, uint8_t *sk, size_t sk_len, uint8_t *pk);

size_t sign(void *sig, size_t sig_len, int sig_type, const void *sk, size_t sk_len, const void *pk, size_t pk_len, const void *msg, size_t msg_len);
int verify(const void *sig, size_t sig_len, int sig_type, const void *pk, size_t pk_len, const void *msg, size_t msg_len);

#endif
