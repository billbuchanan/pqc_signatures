#include "matrix.h"

void matrix_print(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t M[nrows][ncols]) {
    MATRIX_SIZE_T i, j;
    for (i=0; i<nrows; i++) {
        for (j=0; j<ncols; j++)
            printf("%02x ",M[i][j]);
        printf("\n");
    }
}

void matrix_print_vec(MATRIX_SIZE_T nrows, uint8_t v[nrows]) {
    matrix_print(1,nrows,(uint8_t (*)[nrows]) v);
}

void matrix_copy(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t B[nrows][ncols], uint8_t A[nrows][ncols]) {
    MATRIX_SIZE_T i, j;
    for (i=0; i<nrows; i++)
        for (j=0; j<ncols; j++)
            B[i][j] = A[i][j];
}

void matrix_transpose(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t B[nrows][ncols], uint8_t A[ncols][nrows]) {
    MATRIX_SIZE_T i, j;
    for (i=0; i<nrows; i++)
        for (j=0; j<ncols; j++)
            B[i][j] = A[j][i];
}

void matrix_add(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t C[nrows][ncols], uint8_t A[nrows][ncols], uint8_t B[nrows][ncols]) {
    MATRIX_SIZE_T i, j;
    for (i=0; i<nrows; i++)
        for (j=0; j<ncols; j++)
            C[i][j] = field_add(A[i][j], B[i][j]);
}

void matrix_mul(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, MATRIX_SIZE_T nlayers, uint8_t C[nrows][ncols], uint8_t A[nrows][nlayers], uint8_t B[nlayers][ncols],unsigned char add) {
    MATRIX_SIZE_T i, j, k;
    for (i=0; i<nrows; i++)
        for (j=0; j<ncols; j++) {
            if (add == MATRIX_ASSIGN)
                C[i][j] = 0;
            for (k=0; k<nlayers; k++)
                C[i][j] = field_add(C[i][j], field_mul(A[i][k], B[k][j]));
        }
}

void matrix_mul_transpose(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, MATRIX_SIZE_T nlayers, uint8_t C[nrows][ncols], uint8_t A[nrows][nlayers], uint8_t B_transpose[ncols][nlayers], unsigned char add) {
    MATRIX_SIZE_T i, j, k;
    for (i=0; i<nrows; i++)
        for (j=0; j<ncols; j++) {
            if (add == MATRIX_ASSIGN)
                C[i][j] = 0;
            for (k=0; k<nlayers; k++)
                C[i][j] = field_add(C[i][j], field_mul(A[i][k], B_transpose[j][k]));
        }
}

void matrix_mul_vec(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t y[nrows], uint8_t A[nrows][ncols], uint8_t x[ncols]) {
    matrix_mul(nrows,1,ncols,(uint8_t (*)[1]) y,A,(uint8_t (*)[1]) x,MATRIX_ASSIGN);
}

void matrix_swap_rows(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t A[nrows][ncols], MATRIX_SIZE_T i, MATRIX_SIZE_T j, MATRIX_SIZE_T col_start) {
    MATRIX_SIZE_T k;
    uint8_t t;
    for (k=col_start; k<ncols; k++) {
        t = A[i][k];
        A[i][k] = A[j][k];
        A[j][k] = t;
    }
}

void matrix_add_row_mul(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t A[nrows][ncols], MATRIX_SIZE_T i, MATRIX_SIZE_T j, uint8_t lambda, MATRIX_SIZE_T col_start) {
    MATRIX_SIZE_T k;
    for (k=col_start; k<ncols; k++)
        A[i][k] = field_add(A[i][k], field_mul(A[j][k],lambda));
}

int matrix_pivot(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t A[nrows][ncols]) {
    MATRIX_SIZE_T i, j;
    MATRIX_SIZE_T row = 0, col = 0;
    uint8_t inv;
    while (col < ncols) {
        if (A[row][col] == 0) {
            for (i=row;i<nrows;i++) {
                if (A[i][col]) {
                    matrix_swap_rows(nrows,ncols,A,row,i,col);
                    break;
                }
            }
            if (i==nrows) {
                col++;
                continue;
            }
        }
        inv = field_inv(A[row][col]);
        for (j=col;j<ncols;j++)
            A[row][j] = field_mul(A[row][j], inv);
        for (i=row+1;i<nrows;i++)
            if (A[i][col])
                matrix_add_row_mul(nrows,ncols,A,i,row,A[i][col],col);
        row++;
        col++;
        if (row==nrows)
            return 0;
    }
    return -1;
}

int matrix_solve(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t x[ncols], uint8_t A[nrows][ncols], uint8_t y[nrows]) {
    MATRIX_SIZE_T i, j, k;
    uint8_t M[nrows][ncols+1];
    for (i=0; i<nrows; i++) {
        for (j=0; j<ncols; j++)
            M[i][j] = A[i][j];
        M[i][ncols] = y[i];
    }
    
    matrix_pivot(nrows,ncols+1,M);
    
    for (i=nrows; i-- > 0;) {
        for (j=0;j<ncols;j++) {
            if (M[i][j]) {
                x[j] = M[i][ncols];
                for (k=j+1;k<ncols;k++)
                    x[j] = field_add(x[j], field_mul(M[i][k],x[k]));
                break;
            }
        }
        if ((j==ncols) && (M[i][ncols]))
            return -1;
    }
    
    return 0;
}

void matrix_lower_to_full(MATRIX_SIZE_T dim, uint8_t matrix[dim][dim], uint8_t *lower) {
    unsigned int j, k;
    for (j=0;j<dim;j++)
        for (k=0;k<dim;k++)
            if (k<=j)
                matrix[j][k] = *(lower++);
            else
                matrix[j][k] = 0;
}

void matrix_full_to_lower(MATRIX_SIZE_T dim, uint8_t *lower, uint8_t matrix[dim][dim]) {
    unsigned int j, k;
    for (j=0;j<dim;j++)
        for (k=0;k<=j;k++)
                *(lower++) = matrix[j][k];
}

void matrix_sym(MATRIX_SIZE_T dim, uint8_t matrix[dim][dim]) {
    unsigned int j, k;
    for (j=0;j<dim;j++)
        for (k=0;k<j;k++) {
            matrix[j][k] ^= matrix[k][j];
            matrix[k][j] = 0;//probably not needed
        }
}

void matrix_add_own_transpose(MATRIX_SIZE_T dim, uint8_t matrix[dim][dim]) {
    unsigned int j, k;
    for (j=0;j<dim;j++)
        for (k=0;k<=j;k++) {
            matrix[j][k] ^= matrix[k][j];
            matrix[k][j] = matrix[j][k];
        }
}

uint8_t matrix_lower_bilin(MATRIX_SIZE_T dim, uint8_t *lower, uint8_t v[dim]) {
    unsigned int j, k;
    uint8_t ret = 0;
    for (j=0;j<dim;j++)
        for (k=0;k<=j;k++)
                ret = field_add(ret, field_mul( field_mul(*(lower++),v[j]), v[k]));
    return ret;
}

uint8_t matrix_full_bilin(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t M[nrows][ncols], uint8_t x[nrows], uint8_t y[ncols]) {
    unsigned int j, k;
    uint8_t ret = 0;
    for (j=0;j<nrows;j++)
        for (k=0;k<ncols;k++)
                ret = field_add(ret, field_mul( field_mul(M[j][k],x[j]), y[k]));
    return ret;
}
