#include "serializer.h"
#include "api.h"
#include <string.h>

void EncodePublicKey(const fmpz_mod_mat_t pub_key, unsigned char *pk){
    for (int i = 0; i < N; i++){
        for (int j = 0; j < PK_ROW_SIZE ; j++){
            pk[i*PK_ROW_SIZE + j] = (int) fmpz_get_ui(fmpz_mod_mat_entry(pub_key, i, j));
        }
    }

    //printf("Encoded pub\n");
}

void DecodePublicKey(fmpz_mod_mat_t pub_key, const unsigned char *pk){
    for (int i = 0; i < N; i++){
        for (int j = 0; j < PK_ROW_SIZE; j++){
            fmpz_set_ui(fmpz_mod_mat_entry(pub_key, i, j), (unsigned int) pk[i*PK_ROW_SIZE + j]);
        }
    }

    //printf("Decoded pub\n");
}

void EncodePrivateKey(const fmpz_mod_mat_t inv_T, const fmpz_mod_mat_t inv_S, unsigned char *sk){
    unsigned long step = 0;
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            sk[i*N + j] = (int) fmpz_get_ui(fmpz_mod_mat_entry(inv_T, i, j));
        }
    }

    step = N*N;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            sk[i*N + j + step] = (int) fmpz_get_ui(fmpz_mod_mat_entry(inv_S, i, j));
        }
    }

    //printf("Encoded priv\n");
}

void DecodePrivateKey(fmpz_mod_mat_t inv_T, fmpz_mod_mat_t inv_S, const unsigned char *sk){
    unsigned long step = 0;
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fmpz_set_ui(fmpz_mod_mat_entry(inv_T, i, j), (unsigned int) sk[i*N + j]);
        }
    }

    step = N*N;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fmpz_set_ui(fmpz_mod_mat_entry(inv_S, i, j), (unsigned int) sk[i*N + j + step]);
        }
    }

    //printf("Decoded priv\n");
}