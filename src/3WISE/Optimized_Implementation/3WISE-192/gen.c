#include "gen.h"
#include "api.h"

void KroneckerProductVec(const fmpz *v1, fmpz *tens_v1_v1_v1){
    fmpz_t val, val2;
    fmpz_init(val); fmpz_init(val2);
    unsigned int pos = 0;
	for (int i = 0; i < N; i++){
        for (int j = i; j < N; j++){
            for (int k = j; k < N; k++){
                fmpz_mul(val, &v1[j], &v1[k]);
                fmpz_mul(val2, val, &v1[i]);
                fmpz_mod(val2, val2, p);
                fmpz_set(&tens_v1_v1_v1[pos++], val2);
            }
        }
    }
    fmpz_clear(val);
    fmpz_clear(val2);
}

void FaceSplitProd(const fmpz_mod_mat_t M1, fmpz_mod_mat_t prod){
    fmpz_t val1, val2, val3;
    fmpz_mod_mat_t tmp;
    fmpz_mod_mat_init(tmp, N, N*N, p);
    fmpz_init(val1);
    fmpz_init(val2);
    fmpz_init(val3);

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fmpz_mod_mat_get_entry(val1, M1, i, j);
            // quadratic
            for (int k = 0; k < N; k++){
                fmpz_mod_mat_get_entry(val2, M1, i, k);
                fmpz_mul(val3, val1, val2);
                fmpz_mod(val3, val3, p);
                fmpz_mod_mat_set_entry(tmp, i, j*N + k, val3);
            }
        }
    }

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fmpz_mod_mat_get_entry(val1, M1, i, j);
            // cubic
            for (int k = 0; k < N*N; k++){
                fmpz_mod_mat_get_entry(val2, tmp, i, k);
                fmpz_mul(val3, val1, val2);
                fmpz_mod(val3, val3, p);
                fmpz_mod_mat_set_entry(prod, i, j*N*N + k, val3);
            }
        }
    }

    fmpz_clear(val1);
    fmpz_clear(val2);
    fmpz_clear(val3);
    fmpz_mod_mat_clear(tmp);
}

void AdjustPublicKey(const fmpz_mod_mat_t pub_key, fmpz_mod_mat_t short_pk){
    fmpz_t val_sum, sum1, sum2, sum3, tmp; fmpz_init(val_sum); fmpz_init(sum1); fmpz_init(sum2); fmpz_init(sum3); fmpz_init(tmp);
    for (int i = 0; i < N; i++){
        unsigned int pos = 0;
        for (int j = 0; j < N; j++){
            for (int k = j; k < N; k++){
                for (int l = k; l < N; l++){
                    fmpz_set_ui(val_sum, 0);
                    if (j == k && k == l){ // diag case
                        fmpz_set(val_sum, fmpz_mod_mat_entry(pub_key, i, j*N*N + j*N + j));
                    }else{
                        if (j == k){
                            fmpz_add(sum1, fmpz_mod_mat_entry(pub_key, i, j*N*N + j*N + l), fmpz_mod_mat_entry(pub_key, i, j*N*N + l*N + j)); // jkl + jlk (jjl + jlj)
                            fmpz_set(sum2, fmpz_mod_mat_entry(pub_key, i, l*N*N + j*N + j)); // ljk (ljj)
                            fmpz_add(val_sum, sum1, sum2);
                        }else if (k == l){
                            fmpz_add(sum1, fmpz_mod_mat_entry(pub_key, i, j*N*N + k*N + k), fmpz_mod_mat_entry(pub_key, i, k*N*N + j*N + k)); // jkl + kjl (jkk + kjk)
                            fmpz_set(sum2, fmpz_mod_mat_entry(pub_key, i, k*N*N + k*N + j)); // klj (kkj)
                            fmpz_add(val_sum, sum1, sum2);
                        }else{
                            // sum of 6 permutations of {j,k,l}
                            fmpz_add(sum1, fmpz_mod_mat_entry(pub_key, i, j*N*N + k*N + l), fmpz_mod_mat_entry(pub_key, i, j*N*N + l*N + k)); // jkl + jlk
                            fmpz_add(sum2, fmpz_mod_mat_entry(pub_key, i, k*N*N + j*N + l), fmpz_mod_mat_entry(pub_key, i, k*N*N + l*N + j)); // kjl + klj
                            fmpz_add(sum3, fmpz_mod_mat_entry(pub_key, i, l*N*N + k*N + j), fmpz_mod_mat_entry(pub_key, i, l*N*N + j*N + k)); // lkj + ljk
                            fmpz_add(tmp, sum1, sum2);
                            fmpz_add(val_sum, tmp, sum3);
                        }
                    }
                    fmpz_mod(val_sum, val_sum, p);
                    fmpz_mod_mat_set_entry(short_pk, i, pos++, val_sum);
                }
            }
        }
    }
    fmpz_clear(val_sum);
}

void GeneratePublicKey(fmpz_mod_mat_t pub_key, fmpz_mod_mat_t inv_T, fmpz_mod_mat_t inv_S){
    fmpz_mod_mat_t T, S, tens_S, tmp_key;

    fmpz_mod_mat_init(tmp_key, N, N*N*N, p);
    fmpz_mod_mat_init(T, N, N, p);
    fmpz_mod_mat_init(S, N, N, p);
    fmpz_mod_mat_init(tens_S, N, N*N*N, p);
    do{
        fmpz_mod_mat_randtest(S, state);
    }while(fmpz_mod_mat_rank(S) < N);

    // cubic face-split product (S . S . S)
    FaceSplitProd(S, tens_S);

    do{
        fmpz_mod_mat_randtest(T, state);
    }while(fmpz_mod_mat_rank(T) < N);
    
    fmpz_mod_mat_mul(tmp_key, T, tens_S);
    AdjustPublicKey(tmp_key, pub_key);

    fmpz_mod_mat_inv(inv_T, T);
    fmpz_mod_mat_inv(inv_S, S);

    fmpz_mod_mat_clear(T);
    fmpz_mod_mat_clear(S);
    fmpz_mod_mat_clear(tens_S);
    fmpz_mod_mat_clear(tmp_key);

    
}