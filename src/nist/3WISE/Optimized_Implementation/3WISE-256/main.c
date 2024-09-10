#include <flint/flint.h>
#include <flint/fmpz_mod_mat.h>
#include <flint/fq.h>

#define P 17
#define N 64

fmpz_t p;

void KroneckerProductVec(const fmpz *v1, const fmpz *v2, fmpz *tens_v1_v2, int v1_len){
    fmpz_t val;
    fmpz_init(val);
	for (int i = 0; i < v1_len; i++){ 
        for (int j = 0; j < N; j++){
            fmpz_mul(val, &v1[i], &v2[j]);
            fmpz_mod(val, val, p);
            fmpz_set(&tens_v1_v2[i*N + j], val);
        }
    }
    fmpz_clear(val);
}

void FaceSplitProd(fmpz_mod_mat_t M1, fmpz_mod_mat_t M2, fmpz_mod_mat_t prod){
    fmpz_t val1, val2, val3;
    fmpz_init(val1);
    fmpz_init(val2);
    fmpz_init(val3);
    int m1_cols = fmpz_mod_mat_ncols(M1);
    int m2_cols = fmpz_mod_mat_ncols(M2);
    for (int i = 0; i < N; i++){
        for (int j = 0; j < m1_cols; j++){
            fmpz_mod_mat_get_entry(val1, M1, i, j);
            for (int k = 0; k < m2_cols; k++){
                fmpz_mod_mat_get_entry(val2, M2, i, k);
                fmpz_mul(val3, val1, val2);
                fmpz_mod(val3, val3, p);
                fmpz_mod_mat_set_entry(prod, i, j*m2_cols + k, val3);
            }
        }
    }
    fmpz_clear(val1);
    fmpz_clear(val2);
    fmpz_clear(val3);
}

int main(int argc, char **argv){
    flint_rand_t state;
    flint_randinit(state);
    flint_randseed(state, (unsigned long)time(NULL) , (unsigned long)time(NULL) );

    fmpz_init(p); fmpz_set_ui(p, P);
    
    fmpz_mod_mat_t inv_T, inv_S, T, S, pub_key, tens_S, tmp;
    fmpz_mod_mat_init(T, N, N, p);
    fmpz_mod_mat_init(S, N, N, p);
    fmpz_mod_mat_init(inv_T, N, N, p);
    fmpz_mod_mat_init(inv_S, N, N, p);
    fmpz_mod_mat_init(pub_key, N, N*N*N, p);
    fmpz_mod_mat_init(tmp, N, N*N, p);
    fmpz_mod_mat_init(tens_S, N, N*N*N, p);

    fmpz_mod_mat_randrank(T, state, N);
    fmpz_mod_mat_randrank(S, state, N);

    // cubic katri rao product (S . S . S)
    // (S.S).S so first (S.S) then each row with S

    FaceSplitProd(S, S, tmp);
    FaceSplitProd(tmp, S, tens_S);

    fmpz_mod_mat_mul(pub_key, T, tens_S);

    fmpz_mod_mat_inv(inv_T, T);
    fmpz_mod_mat_inv(inv_S, S);

    fmpz *message = _fmpz_vec_init(N);
    fmpz *signature = _fmpz_vec_init(N);
    fmpz *tmp_vec = _fmpz_vec_init(N);
    fmpz *in_vec = _fmpz_vec_init(N*N*N);
    fmpz *tens_v = _fmpz_vec_init(N*N);

    _fmpz_vec_randtest_unsigned(message, state, N, 4);
    
    fmpz_mod_mat_mul_fmpz_vec(tmp_vec, inv_T, message, N);

    fmpz_mod_ctx_t ord_ctx, p_ctx;
    fmpz_t order; fmpz_init(order); fmpz_set_ui(order, P-1);
    fmpz_t exp; fmpz_init(exp); fmpz_set_ui(exp, 3);
    fmpz_t invexp; fmpz_init(invexp);
    fmpz_mod_ctx_init(ord_ctx, order);
    fmpz_mod_ctx_init(p_ctx, p);
    fmpz_mod_inv(invexp, exp, ord_ctx);

    for (int i = 0; i < N; i++)
        fmpz_mod_pow_fmpz(&tmp_vec[i], &tmp_vec[i], invexp, p_ctx);
    
    fmpz_mod_mat_mul_fmpz_vec(signature, inv_S, tmp_vec, N);

    printf("message: "); _fmpz_vec_print(message, N); printf("\n");
    printf("signature: "); _fmpz_vec_print(signature, N); printf("\n");

    KroneckerProductVec(signature, signature, tens_v, N);
    KroneckerProductVec(tens_v, signature, in_vec, N*N);

    fmpz_mod_mat_mul_fmpz_vec(signature, pub_key, in_vec, N*N*N);

    printf("message: "); _fmpz_vec_print(signature, N); printf("\n");

    fmpz_mod_mat_clear(T);
    fmpz_mod_mat_clear(S);
    fmpz_mod_mat_clear(inv_T);
    fmpz_mod_mat_clear(inv_S);
    fmpz_mod_mat_clear(pub_key);
    fmpz_mod_mat_clear(tmp);
    fmpz_mod_mat_clear(tens_S);

    _fmpz_vec_clear(message, N);
    _fmpz_vec_clear(signature, N);
    _fmpz_vec_clear(tmp_vec, N);
    _fmpz_vec_clear(tens_v, N*N);
    _fmpz_vec_clear(in_vec, N*N*N);

    fmpz_clear(order);
    fmpz_clear(exp);
    fmpz_clear(invexp);
    
    fmpz_mod_ctx_clear(ord_ctx);
    fmpz_mod_ctx_clear(p_ctx);

}