#include "gen.h"
#include "serializer.h"
#include "api.h"
#include "rng.h"

extern fmpz_t p;
extern fq_ctx_t ctx;
extern flint_rand_t state;

void GenRndMat(mzd_t* mat){
    mzd_t *row = mzd_init(N*N, 1), *tmp = mzd_init(N, N);
    rci_t rank = 0;
    unsigned char vals[N*N / 8] = {0};

    do{
        randombytes(vals, sizeof(vals));
        uchar_to_mzd(vals, row, sizeof(vals));
        for (int i = 0; i < N*N; i++){
            int q = i/N;
            int r = i%N;
            mzd_write_bit(tmp, q, r, mzd_read_bit(row, i, 0));
        }
        mzd_copy(mat, tmp);
        rank = mzd_gauss_delayed(tmp, 0, 1);
    }while(rank < N);

    mzd_free(tmp);
    mzd_free(row);
}

void GenVandermonde(fq_mat_t mat){

    // Highly optimized vandermode matrix calculation of monomials t^(i*(p^j))
    // Here the Matrix is of dimension (N*(d+1) since the coefficient vector is a d-th degree linpoly (d+1 coeffs)
    // In every row exponentiate to p^j precompute all monomials t^(p^j) in an array
    // Use that array in every row to just exponentiate to precomp[j]^i
    // Besides it checks for every i-th row where i is divisible (on 1-index pos)
    // When divisibility holds just exponentiate every entry of the i/r row to the exponent r  

    fq_t precomp[d+1];
    fq_t poly;
    fq_t entry;
    fmpz_t exp;
    fmpz_init(exp);
    fq_init(entry, ctx);
    fq_init(poly, ctx);
    fmpz_poly_set_coeff_ui(poly, 1, 1); // monomial t

    for (int i = 0; i <= d; i++){
        fq_init(precomp[i], ctx);
        fmpz_pow_ui(exp, p, i); // t^(p^i)
        fq_pow(precomp[i], poly, exp, ctx);
    }
    for (int i = 0; i < N; i++){
        for (int j = 0; j <= d; j++){
            if (i > 2 && i % 2 == 0){
                fq_struct *val = fq_mat_entry(mat, i/2, j);
                fq_pow_ui(entry, val, 2, ctx);
            }else if (i > 3 && i % 3 == 0){
                fq_struct *val = fq_mat_entry(mat, i/3, j);
                fq_pow_ui(entry, val, 3, ctx);
            }else if (i > 5 && i % 5 == 0){
                fq_struct *val = fq_mat_entry(mat, i/5, j);
                fq_pow_ui(entry, val, 5, ctx);
            }else{
                fq_pow_ui(entry, precomp[j], i, ctx); // t^((p^j)*i)
            }
            fq_mat_entry_set(mat, i, j, entry, ctx);   
        }
    }

    fq_clear(poly, ctx);
    fq_clear(entry, ctx);
    fmpz_clear(exp);

    for (int i = 0; i <= d; i++)
        fq_clear(precomp[i], ctx);

}

void Ident(mzd_t *mat){
    for (int i = 0; i < N; i++){
        mzd_write_bit(mat, i, i, 1);
    }
}

void GenRndLinPermPolyMat(mzd_t *mat, fq_poly_t private_polynomial, fq_mat_t vander){

    // This method does two things mainly
    // Calculates the matrix representation of the Linearised Permutation Polynomial L2
    // Calculates the private polynomial (poly instead of matrix) so G(X) = X.l2(X)
    // MatrixRep L2 is used for building pubkey P(X) by using Tensor Algebra
    // PolyRep of G(X) is used for signing to find roots of P(X) - v - H(m) = 0 (Trapdoor function)

    slong rank = 0;
    fmpz_mod_mat_t tmp;
    fmpz_mod_mat_init(tmp, N, N, p);
    fmpz_mod_ctx_t mctx;
    fmpz_mod_ctx_init(mctx, p);

    fmpz_t coeff;
    fmpz_init(coeff);

    // Generate G(X)=X*l2(X) of degree (q^d)+1
    // Store every coeff of G(X) in a vector to mult by vandermonde matrix and find the matrix rep of L2

    fq_struct *vec = _fq_vec_init(d+1, ctx), *out = _fq_vec_init(N, ctx);
    do{
        for (int i = 0; i <= d; i++){
            fmpz_mod_poly_t el;
            fmpz_mod_poly_init(el, mctx);
            fmpz_poly_t pol;
            fmpz_poly_init(pol);
            long exp = powl(2,i)+1;
            if (i < d){
                fmpz_mod_poly_randtest_monic(el, state, N, mctx);
                fmpz_mod_poly_get_fmpz_poly(pol, el, mctx);
                fq_poly_set_coeff(private_polynomial, exp, pol, ctx);
            }else if (i == d){ //monomial X^(2^d+1) is monic so has coefficient = 1
                fmpz_mod_poly_set_ui(el, 1, mctx);
                fmpz_mod_poly_get_fmpz_poly(pol, el, mctx);
                fq_poly_set_coeff(private_polynomial, exp, pol, ctx);
            }
            fq_set_fmpz_poly(vec + i, pol, ctx);
            fmpz_mod_poly_clear(el, mctx);
            fmpz_poly_clear(pol);
        }

        fq_mat_mul_vec(out, vander, vec, d+1, ctx);

        for (int i = 0; i < N; i++){
            fmpz_poly_t poly;
            fmpz_poly_init(poly);
            fq_get_fmpz_poly(poly, out + i, ctx);
            for (int j = 0; j < N; j++){
                fmpz_poly_get_coeff_fmpz(coeff, poly, j);
                fmpz_mod_mat_set_entry(tmp, j, i, coeff);
            }
            fmpz_poly_clear(poly);
        }
        rank = fmpz_mod_mat_rank(tmp);
        
    }while(rank < N);

    // Vander dot coeffs = [l(t), ..., l(t^n)] where "l(x)" is a linearised permutation poly (rank = n)
    // convert to matrix L where each column is the image of poly l1(t^j)
    // then l(t^i) = L(e_i) for every canonical vector by linearity
    // on variable "mat" we'll have matrix representation of the LinPermPoly L2

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fmpz_mod_mat_get_entry(coeff, tmp, i, j);
            if (fmpz_get_ui(coeff) == 1)
                mzd_write_bit(mat, i, j, 1);
        }
    }

    fmpz_clear(coeff);
    fmpz_mod_ctx_clear(mctx);
    fmpz_mod_mat_clear(tmp);
    _fq_vec_clear(vec, d+1, ctx);
    _fq_vec_clear(out, N, ctx);
}

void GenerateBasis(mzd_t *mat){

    // Matrix variable "mat" holds the Finite Field multiplication representation used in Tensor Algebra
    // the C_f(x) is the Companion matrix of the irreducible polynomial f(x)
    // So M = [C_f(x)^0,...,C_f(x)^(n-1)] is the representation F2^n = F2 / <f(x)>

    mzd_t *companion = mzd_init(N, N);

    for (int i = 1; i < N; i++){
        mzd_write_bit(companion, i, i-1, 1);
    }

    fmpz_mod_ctx_t mctx;
    fmpz_mod_ctx_init(mctx, p);

    fmpz_t coeff;
    fmpz_init(coeff);
    for (int i = 0; i < N; i++){    
        fmpz_mod_poly_get_coeff_fmpz(coeff, ctx->modulus, i, mctx);
        if (fmpz_get_ui(coeff) == 1){
            mzd_write_bit(companion, i, N-1, 1);
        }
    }

    mzd_t *cur = mzd_init(N, N), *tmp = mzd_init(N, N);
    Ident(cur);
    for (int i = 0; i < N; i++){ // map cur to M
        for (int j = 0; j < N; j++){
            for (int k = 0; k < N; k++){
                if (mzd_read_bit(cur, j, k) == 1)
                    mzd_write_bit(mat, j, i*N + k, 1);
            }
        }
        #ifdef MULT_STRASSEN
            mzd_mul(tmp, cur, companion, N);
        #else
            mzd_mul_naive(tmp, cur, companion);
        #endif
        mzd_copy(cur, tmp);
    }

    fmpz_clear(coeff);
    fmpz_mod_ctx_clear(mctx);
    mzd_free(cur);
    mzd_free(tmp);
    mzd_free(companion);
    
}

void AdjustPublicKey(mzd_t *pub_key, mzd_t *short_pk){

    // This method does two major things
    // First it rewrites every row of pubkey as a quadratic form
    // Then it sums that quadratic form with the transpose of itself
    // The resulting quadform is converted back to a n*n vector and inserted at pubkey
    // Then the pubkey is converted from a n*n matrix to a n*(n*(n+1))/2 matrix
    // It is done by eliminating redundancies inserted by tensor products
    // This is the minimal representation of the cryptographic scheme's public key

    for (int i = 0; i < N; i++){
        unsigned int pos = 0;
        for (int j = 0; j < N; j++){
            for (int k = j; k < N; k++){
                if (j==k)
                    mzd_write_bit(short_pk, i, pos++, mzd_read_bit(pub_key, i, j*(N+1))); // jj
                else
                    mzd_write_bit(short_pk, i, pos++, mzd_read_bit(pub_key, i, j*N + k) + mzd_read_bit(pub_key, i, k*N + j)); // jk + kj
            }
        }
    }
}

void KroneckerProductMat(const mzd_t *M1, const mzd_t *M2, mzd_t *tens_M1_M2){
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
            if (mzd_read_bit(M1, i, j) == 1){
                for (int m=0; m < N; m++){
		            for (int k = 0; k < N; k++){
                        mzd_write_bit(tens_M1_M2, i*N + m, j*N + k, mzd_read_bit(M2, m, k) );
                    }
                }
            }
        }
    }
}

// Optimized Kronecker product for vectors as output vector is has length n(n+1)*1/2

void KroneckerProductVec(const mzd_t *v1, const mzd_t *v2, mzd_t *tens_v1_v2){
    int pos = 0;
	for (int i = 0; i < N; i++){ 
        for (int j = i; j < N; j++){ // 
            mzd_write_bit(tens_v1_v2, pos, 0, mzd_read_bit(v1, i, 0)*mzd_read_bit(v2, j, 0));
            pos++;
        }
    }
}

void GenPublicKey(mzd_t *pub_key, const mzd_t *M, const mzd_t *T, const mzd_t *S, const mzd_t *L1, const mzd_t *L2){

    // This method Generates the public key P(X)
    // P = T.M.(L1 X (L2.L1)).(S X S) = T.M.((L1.S) X (L2.L1.S))
    // "." stands for dot product and "X" for tensor/kronecker product

    mzd_t *L2L1_S = mzd_init(N, N),
    *L1_S = mzd_init(N, N),
    *L2_L1 = mzd_init(N, N),
    *T_M = mzd_init(N, N*N),
    *tens_F_S = mzd_init(N*N, N*N);

    #ifdef MULT_STRASSEN
        mzd_mul(L2_L1, L2, L1, N);
        mzd_mul(L2L1_S, L2_L1, S, N);
        mzd_mul(L1_S, L1, S, N);
        mzd_mul(T_M, T, M, N);
    #else
        mzd_mul_naive(L2_L1, L2, L1);
        mzd_mul_naive(L2L1_S, L2_L1, S);
        mzd_mul_naive(L1_S, L1, S);
        mzd_mul_naive(T_M, T, M);
    #endif

    KroneckerProductMat(L1_S, L2L1_S, tens_F_S);

    // M4RI lib has Strassen optimized method for matrix multiplication
    // Might run in multi-thread. Enable by compiling with Makefile option
    #ifdef MULT_STRASSEN
    mzd_mul(pub_key, T_M, tens_F_S, N);
    #else
    mzd_mul_naive(pub_key, T_M, tens_F_S);
    #endif

    mzd_free(tens_F_S);
    mzd_free(L1_S);
    mzd_free(L2L1_S);
    mzd_free(L2_L1);
    mzd_free(T_M);

}
