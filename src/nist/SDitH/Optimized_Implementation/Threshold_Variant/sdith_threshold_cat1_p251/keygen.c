#include "keygen.h"

#include <stdio.h>
#include "rnd.h"

int sdith_keygen_internal(sdith_public_key_t* pk, sdith_secret_key_t* sk) {
    // Sample the master seed, a seed to sample H and the secret key
    uint8_t master_seed[PARAM_SEED_SIZE];
    sample_seed(master_seed);
    prg_context entropy_ctx;
    prg_init(&entropy_ctx, master_seed, NULL);
    samplable_t entropy = prg_to_samplable(&entropy_ctx);

    generate_instance_with_solution(&sk->inst, &sk->wit, &entropy);
    pk->inst = sk->inst;
    return 0;
}

int sdith_validate_keys_internal(const sdith_public_key_t* pk, const sdith_secret_key_t* sk) {
    // Check the consistency between PK and SK
    if(pk != NULL) {
        if(!are_same_instances(pk->inst, sk->inst)) {
            printf("Error: Values from PK are not consistent with SK.\n");
            return -1;
        }
    }

    // Check the consistency of the SK
    return (is_correct_solution(sk->inst, sk->wit) == 0);
}

int sdith_free_keys_internal(sdith_public_key_t* pk, sdith_secret_key_t* sk) {
    if(sk != NULL) {
        free_instance_solution(sk->wit);
        free_instance(sk->inst);
    }
    if(pk != NULL) {
        if(sk == NULL || (pk->inst != sk->inst))
            free_instance(pk->inst);
    }
    return 0;
}

int deserialize_public_key(sdith_public_key_t* key, const uint8_t* buf, size_t buflen) {
    if(key == NULL || buf == NULL)
        return -1;
    
    size_t bytes_expected = PARAM_INSTANCE_SIZE;
    if (buflen < bytes_expected)
        return -1;

    key->inst = deserialize_instance(buf);
    return 0;
}

int serialize_public_key(uint8_t* buf, const sdith_public_key_t* key, size_t buflen) {
    if(key == NULL || buf == NULL)
        return -1;
    
    size_t bytes_required = PARAM_INSTANCE_SIZE;
    if (buflen < bytes_required)
        return -1;

    serialize_instance(buf, key->inst);
    return (int) bytes_required;
}

int deserialize_secret_key(sdith_secret_key_t* key, const uint8_t* buf, size_t buflen) {
    if(key == NULL || buf == NULL)
        return -1;
    
    size_t bytes_expected = PARAM_INSTANCE_SIZE + PARAM_SOL_SIZE;
    if (buflen < bytes_expected)
        return -1;

    key->inst = deserialize_instance(buf);
    key->wit = deserialize_instance_solution(buf + PARAM_INSTANCE_SIZE);
    return 0;
}

int serialize_secret_key(uint8_t* buf, const sdith_secret_key_t* key, size_t buflen) {
    if(key == NULL || buf == NULL)
        return -1;
    
    size_t bytes_required = PARAM_INSTANCE_SIZE + PARAM_SOL_SIZE;
    if (buflen < bytes_required)
        return -1;

    serialize_instance(buf, key->inst);
    serialize_instance_solution(buf + PARAM_INSTANCE_SIZE, key->wit);
    return (int) bytes_required;
}

int sdith_keygen(unsigned char *pk, unsigned char *sk) {
    sdith_public_key_t ppk;
    sdith_secret_key_t ssk;
    int ret = sdith_keygen_internal(&ppk, &ssk);
    if(ret) {
        return ret;
    }
    ret = serialize_public_key(pk, &ppk, PARAM_INSTANCE_SIZE);
    if(ret < 0) {
        return ret;
    }
    ret = serialize_secret_key(sk, &ssk, PARAM_INSTANCE_SIZE + PARAM_SOL_SIZE);
    if(ret < 0) {
        return ret;
    }

#ifndef NDEBUG
    // In debug mode, let us check if the key generation
    //    produces valid key pair. 
    ret = sdith_validate_keys(pk, sk);
    if(ret)
        printf("Error: Pair (PK, SK) invalid.\n");
#endif

    sdith_free_keys_internal(&ppk, &ssk);
    return 0;
}

int sdith_validate_keys(const unsigned char *pk, const unsigned char *sk) {
    int ret;

    // Deserialize Secret Key
    sdith_secret_key_t ssk;
    ret = deserialize_secret_key(&ssk, sk, PARAM_INSTANCE_SIZE + PARAM_SOL_SIZE);
    if(ret < 0)
        return -1;

    // Serialize Secret Key
    sdith_public_key_t ppk;
    if(pk != NULL) {
        ret = deserialize_public_key(&ppk, pk, PARAM_INSTANCE_SIZE);
        if (ret < 0) {
            sdith_free_keys_internal(NULL, &ssk);
            return -1;
        }
    }

    // Validate the key(s)
    if(pk != NULL) {
        ret = sdith_validate_keys_internal(&ppk, &ssk);
        sdith_free_keys_internal(&ppk, &ssk);
    } else {
        ret = sdith_validate_keys_internal(NULL, &ssk);
        sdith_free_keys_internal(NULL, &ssk);
    }

    return ret;
}

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
    return sdith_keygen(pk, sk);
}

int crypto_sign_valid_keys(unsigned char *pk, unsigned char *sk) {
    return sdith_validate_keys(pk, sk);
}
