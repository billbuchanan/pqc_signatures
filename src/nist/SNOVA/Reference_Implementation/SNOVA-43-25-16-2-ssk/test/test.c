#include <stdio.h>

#include "../deriv_params.h"
#include "../snova.h"
#include "../util/util.h"

int main() {
    snova_init();
    uint8_t array_digest[64];
    uint8_t array_signature1[bytes_signature + bytes_salt];
    uint8_t array_signature2[bytes_signature + bytes_salt];

    uint8_t seed[seed_length];
    uint8_t* pt_private_key_seed;
    uint8_t* pt_public_key_seed;
    uint8_t pk[bytes_pk], sk[bytes_sk];
    uint8_t array_salt[bytes_salt];

    uint8_t entropy_input[48];
    for (int i = 0; i < 48; i++) {
        entropy_input[i] = i;
    }
    randombytes_init(entropy_input, NULL, 256);
    randombytes(seed, seed_length);

    pt_public_key_seed = seed;
    pt_private_key_seed = seed + seed_length_public;

    create_salt(array_salt);

    printf("private key seed (%d bytes): \n", seed_length_private);
    print_byte(pt_private_key_seed, seed_length_private);
    printf("public key seed (%d bytes): \n", seed_length_public);
    print_byte(pt_public_key_seed, seed_length_public);

    printf("generate_keys_pack\n");
    generate_keys_esk(pt_public_key_seed, pt_private_key_seed, pk, sk);
    printf("private key size: (%d bytes): \n", bytes_sk);
    print_byte(sk, bytes_sk);
    printf("public key size: (%d bytes): \n", bytes_pk);
    print_byte(pk, bytes_pk);

    printf("hash: \n");
    randombytes(array_digest, 64);
    print_byte(array_digest, 64);
    printf("=======================\n");

    sign_digest_esk(array_signature1, array_digest, 64, array_salt, sk);
    printf("signature (%d byte): \n", bytes_signature + bytes_salt);
    print_byte(array_signature1, bytes_signature + bytes_salt);

    int r = verify_signture(array_digest, 64, array_signature1, pk);
    if (r == 0) {
        printf("verification successful!\n");
    } else {
        printf("verification failed! err = %d\n", r);
    }

    printf("\nsign_digest_by_seed: \n");
    printf("=======================\n");
    sign_digest_ssk(array_signature2, array_digest, 64, array_salt, seed);
    printf("signature (%d byte): \n", bytes_signature + bytes_salt);
    print_byte(array_signature2, bytes_signature + bytes_salt);

    r = verify_signture(array_digest, 64, array_signature2, pk);
    if (r == 0) {
        printf("verification successful!\n");
    } else {
        printf("verification failed! err = %d\n", r);
    }

    return 0;
}
