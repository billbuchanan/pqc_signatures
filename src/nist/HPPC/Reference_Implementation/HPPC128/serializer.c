#include "serializer.h"
#include "api.h"

void char2bits2(unsigned char c, unsigned char * bits) {
    for(;c;++bits,c>>=1) *bits = c&1;
}


void uchar_to_mzd(const unsigned char *vec, mzd_t *mzd, int len){ // byte vector to bit vector
    for (int i = 0; i < len; i++){
        unsigned char bits[8] = {0};
        char2bits2(vec[i], bits);
        for (int j = 0; j < 8; j++){
            mzd_write_bit(mzd, i*8 + j, 0, bits[j]);
        }
    }
}

void mzd_to_uchar(const mzd_t *mzd, unsigned char *vec, int len){ // bit vector to byte vector
    for (int i = 0; i < len; i++){
        vec[i] = 0;
        for (int j = 0; j < 8; j++){
            if (mzd_read_bit(mzd, i*8 + j, 0)==1)
                vec[i] += (unsigned char) ((int)pow(2,j));
        }
    }
}

void GetRow(mzd_t *row, const mzd_t *mat, int nrow, int len){ // helper for encoding binary matrix to byte vector
    for (int i = 0; i < len; i++){
       mzd_write_bit(row, i, 0, mzd_read_bit(mat, nrow, i));
    }
}

void SetRow(mzd_t *mat, const mzd_t *row, int nrow, int len){ // helper for decoding byte vector to binary matrix
    for (int i = 0; i < len; i++){
       mzd_write_bit(mat, nrow, i, mzd_read_bit(row, i, 0));
    }
}

void EncodePublicKey(const mzd_t *pub_key, unsigned char *pk){
    int bytes_per_row = PK_ROW_SIZE / 8;
    mzd_t *row = mzd_init(PK_ROW_SIZE, 1);
    long step = 0;
    for (int i = 0; i < N; i++){
        GetRow(row, pub_key, i, PK_ROW_SIZE);
        mzd_to_uchar(row, pk + i*bytes_per_row, bytes_per_row);
    }

    mzd_free(row);
}

void DecodePublicKey(mzd_t *pub_key, const unsigned char *pk){
    int bytes_per_row = PK_ROW_SIZE / 8;
    mzd_t *row = mzd_init(PK_ROW_SIZE, 1);
    long step = 0;
    for (int i = 0; i < N; i++){
        uchar_to_mzd(pk + i*bytes_per_row, row, bytes_per_row);
        SetRow(pub_key, row, i, PK_ROW_SIZE);
    }

    mzd_free(row);
}

void EncodePrivateKey(unsigned char *sk, const mzd_t *inv_T, const mzd_t *inv_S, const mzd_t *inv_L1, fq_poly_t private_polynomial, fq_ctx_t ctx){
    long step = 0;
    int bytes_per_row = N / 8;
    mzd_t *row = mzd_init(N, 1);
    for (int i = 0; i < N; i++){
        GetRow(row, inv_T, i, N);
        mzd_to_uchar(row, sk + i*bytes_per_row, bytes_per_row);
    }

    step = bytes_per_row * N;

    for (int i = 0; i < N; i++){
        GetRow(row, inv_S, i, N);
        mzd_to_uchar(row, sk + i*bytes_per_row + step, bytes_per_row);
    }

    step += bytes_per_row * N;

    for (int i = 0; i < N; i++){
        GetRow(row, inv_L1, i, N);
        mzd_to_uchar(row, sk + i*bytes_per_row + step, bytes_per_row);
    }

    step += bytes_per_row * N;

    // For storing the private fq_poly_t of FLINT
    // first convert to mzd_t* then to uchar
    mzd_t *coeffs = mzd_init((d+1)*N, 1);
    fq_t coeff;
    fq_init(coeff, ctx);
    for (int i = 0; i <= d; i++){
        long exp = powl(2,i) + 1;
        fq_poly_get_coeff(coeff, private_polynomial, exp, ctx); // coef is a value in Fq
        for (int j = 0; j < N; j++){
            if (fmpz_poly_get_coeff_ui(coeff, j) == 1)
                mzd_write_bit(coeffs, i*N + j, 0, 1);
        }
    }

    mzd_to_uchar(coeffs, sk + step, (d+1)*N / 8);

    fq_clear(coeff, ctx);
    mzd_free(row);
    mzd_free(coeffs);
}

void DecodePrivateKey(const unsigned char *sk, mzd_t *inv_T, mzd_t *inv_S, mzd_t *inv_L1, mzd_t *private_polynomial){
    long step = 0;
    int bytes_per_row = N / 8;
    mzd_t *row = mzd_init(N*N, 1);
    for (int i = 0; i < N; i++){
        uchar_to_mzd(sk + i*bytes_per_row, row, bytes_per_row);
        SetRow(inv_T, row, i, N);
    }

    step = bytes_per_row * N;

    for (int i = 0; i < N; i++){
        uchar_to_mzd(sk + i*bytes_per_row + step, row, bytes_per_row);
        SetRow(inv_S, row, i, N);
    }

    step += bytes_per_row * N;

    for (int i = 0; i < N; i++){
        uchar_to_mzd(sk + i*bytes_per_row + step, row, bytes_per_row);
        SetRow(inv_L1, row, i, N);
    }

    step += bytes_per_row * N;

    uchar_to_mzd(sk + step, private_polynomial, (d+1)*N / 8);

    mzd_free(row);
}