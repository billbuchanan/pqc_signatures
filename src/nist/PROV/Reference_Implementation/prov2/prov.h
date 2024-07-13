#ifndef PROV_H
#define PROV_H

#include <stdint.h>
#include <stdlib.h>

#include "field.h"
#include "matrix.h"

#include "SHAKE/hash.h"

typedef struct {
    uint16_t n;
    uint16_t m;
    uint16_t delta;
    uint16_t len_secret_seed;
    uint16_t len_public_seed;
    uint16_t len_salt;
    uint16_t len_hashed_pkey;
} prov_param_t;

typedef struct {
    uint8_t *public_seed;
    uint8_t *secret_seed;
    uint8_t *hashed_pkey;
} prov_skey_t;

typedef struct {
    uint8_t *public_seed;
    uint8_t *secret_seed;
    uint8_t *hashed_pkey;
    uint8_t *aux;
} prov_expanded_skey_t;

typedef struct {
    uint8_t *public_seed;
    uint8_t *p3;
} prov_pkey_t;

typedef struct {
    uint8_t *public_seed;
    uint8_t *p1;
    uint8_t *p2;
    uint8_t *p3;
} prov_expanded_pkey_t;

typedef struct {
    uint8_t *sig;
    uint8_t *salt;
} prov_sig_t;

void prov_alloc_skey(prov_param_t *param, prov_skey_t *skey);
void prov_free_skey(prov_skey_t *skey);

void prov_alloc_expanded_skey(prov_param_t *param, prov_expanded_skey_t *expanded_key);
void prov_free_expanded_skey(prov_expanded_skey_t *expanded_skey);

void prov_alloc_pkey(prov_param_t *param, prov_pkey_t *pkey);
void prov_free_pkey(prov_pkey_t *pkey);

void prov_alloc_expanded_pkey(prov_param_t *param, prov_expanded_pkey_t *expanded_pkey);
void prov_free_expanded_pkey(prov_expanded_pkey_t *expanded_pkey);

void prov_alloc_sig(prov_param_t *param, prov_sig_t *sig);
void prov_free_sig(prov_sig_t *sig);

int prov_keygen(prov_param_t *param, prov_pkey_t *pkey, prov_skey_t *skey);

void prov_expand_secret(prov_param_t *param, prov_expanded_skey_t *expanded_skey, prov_skey_t *skey);

int prov_sign(prov_param_t *param, prov_sig_t *sig, prov_skey_t *skey, const uint8_t *msg, size_t len_msg);

int prov_expanded_sign(prov_param_t *param, prov_sig_t *sig, prov_expanded_skey_t *expanded_skey, const uint8_t *msg, size_t msg_len);

void prov_expand_public(prov_param_t *param, prov_expanded_pkey_t *expanded_pkey, prov_pkey_t *pkey);

int prov_verify(prov_param_t *param, prov_sig_t *sig, prov_pkey_t *pkey, const uint8_t *msg, size_t msg_len);

int prov_expanded_verify(prov_param_t *param, prov_sig_t *sig, prov_expanded_pkey_t *pkey, const uint8_t *msg, size_t len_msg);

void prov_write_pkey(prov_param_t *param,uint8_t *pos,prov_pkey_t *pkey);
void prov_read_pkey(prov_param_t *param,prov_pkey_t *pkey,uint8_t *pos);

void prov_write_expanded_skey(prov_param_t *param,uint8_t *pos,prov_expanded_skey_t *expanded_skey);
void prov_read_expanded_skey(prov_param_t *param,prov_expanded_skey_t *expanded_skey,uint8_t *pos);

void prov_write_sig(prov_param_t *param,uint8_t *pos,prov_sig_t *sig);
void prov_read_sig(prov_param_t *param,prov_sig_t *sig,uint8_t *pos);

#endif /*PROV_H*/
