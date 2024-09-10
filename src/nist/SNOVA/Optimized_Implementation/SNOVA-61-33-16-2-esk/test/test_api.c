#include <stdio.h>

#include "../api.h"
#include "../util/util.h"

#define text_len 999

int main() {
    snova_init();
    uint8_t pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    uint8_t text[text_len] = {0};
    uint8_t sm[CRYPTO_BYTES + text_len];
    uint64_t smlen;
    uint8_t entropy_input[48];
    for (int i = 0; i < 48; i++) {
        entropy_input[i] = i;
    }

    randombytes_init(entropy_input, NULL, 256);

    printf("generate_keys\n");
    crypto_sign_keypair(pk, sk);
    printf("private key size: (%d bytes): \n", CRYPTO_SECRETKEYBYTES);
    print_byte(sk, CRYPTO_SECRETKEYBYTES);
    printf("=======================\n");
    printf("public key size: (%d bytes): \n", CRYPTO_PUBLICKEYBYTES);
    print_byte(pk, CRYPTO_PUBLICKEYBYTES);
    printf("=======================\n");

    randombytes(text, text_len);
    printf("text (%d byte): \n", text_len);
    print_byte(text, text_len);
    printf("=======================\n");
    crypto_sign(sm, &smlen, text, text_len, sk);
    printf("sm gen (%ld byte): \n", smlen);
    print_byte(sm, smlen);
    printf("=======================\n");

    uint32_t mlan;
    uint8_t text1[text_len] = {0};
    int r = crypto_sign_open(text1, &mlan, sm, CRYPTO_BYTES + text_len, pk);
    printf("crypto_sign_open: %d\n", r);
    print_byte(text1, mlan);
    return 0;
}
