#ifndef CONFIG_H
#define CONFIG_H

#define DILITHIUM_MODE 3
#define DILITHIUM_USE_AES
#define DILITHIUM_RANDOMIZED_SIGNING

#define CRYPTO_ALGNAME "Dilithium3-AES-R"
#define DILITHIUM_NAMESPACE(s) pqcrystals_dilithium3aes_r_avx2##s

#endif
