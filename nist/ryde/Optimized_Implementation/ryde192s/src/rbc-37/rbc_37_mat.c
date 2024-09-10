/**
 * \file rbc_37_mat.c
 * \brief Implementation of rbc_37_mat.h
 */

#include "rbc_37_mat.h"




/**
 * \fn void rbc_37_mat_init(rbc_37_mat* m, uint32_t rows, uint32_t columns)
 * \brief This function allocates the memory for a rbc_37_mat.
 *
 * \param[out] m Pointer to the allocated rbc_37_mat
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat
 */
void rbc_37_mat_init(rbc_37_mat* m, uint32_t rows, uint32_t columns) {
    *m = calloc(rows, sizeof(rbc_37_vec *));
    (*m)[0] = calloc(rows * columns, sizeof(rbc_37_elt));
    for(size_t i = 0; i < rows; ++i) {
        (*m)[i] = (*m)[0] + (i * columns);
    }
    if (m == NULL) exit(EXIT_FAILURE);
}




/**
 * \fn void rbc_37_mat_clear(rbc_37_mat m)
 * \brief This function clears a rbc_37_mat element.
 *
 * \param[out] m rbc_37_mat
 */
void rbc_37_mat_clear(rbc_37_mat m) {
    free(m[0]);
    free(m);
}



/**
 * \fn void rbc_37_mat_set_zero(rbc_37_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite elements to zero.
 *
 * \param[out] m rbc_37_mat
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat
 */
void rbc_37_mat_set_zero(rbc_37_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_37_vec_set_zero(m[i], columns);
    }
}



/**
 * \fn void rbc_37_mat_set_random(seedexpander_shake* ctx, rbc_37_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite field elements with random values using NIST seed expander.
 *
 * \param[out] ctx Seed expander
 * \param[out] m rbc_37_mat
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat
 */
void rbc_37_mat_set_random(seedexpander_shake_t* ctx, rbc_37_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_37_vec_set_random(ctx, m[i], columns);
    }
}



/**
 * \fn void rbc_37_mat_vec_mul(rbc_37_vec o, const rbc_37_mat m, const rbc_37_vec v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a matrix of finite field elements by a vector.
 *
 * \param[out] o rbc_37_vec equal to \f$ m \times v \f$
 * \param[in] m rbc_37_mat
 * \param[in] v rbc_37_vec
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat, and Row size of the rbd_vec
 */
void rbc_37_mat_vec_mul(rbc_37_vec o, const rbc_37_mat m, const rbc_37_vec v, uint32_t rows, uint32_t columns) {
    rbc_37_elt tmp, acc;
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_37_elt_set_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_37_elt_mul(tmp, m[i][j], v[j]);
            rbc_37_elt_add(acc, acc, tmp);
        }
        rbc_37_elt_set(o[i], acc);
    }
}



/**
 * \fn void rbc_37_mat_to_string(uint8_t* str, const rbc_37_mat m, uint32_t rows, uint32_t columns)
 * \brief This function parses a matrix of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] m rbc_37_mat
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat
 */
void rbc_37_mat_to_string(uint8_t* str, const rbc_37_mat m, uint32_t rows, uint32_t columns) {
  rbc_37_vec t;
  rbc_37_vec_init(&t, rows * columns);
  for(size_t i = 0 ; i < rows ; i++) {
    for(size_t j = 0 ; j < columns ; j++) {
      rbc_37_elt_set(t[i * columns + j], m[i][j]);
    }
  }
  rbc_37_vec_to_string(str, t, rows * columns);
  rbc_37_vec_clear(t);
}



/**
 * \fn void rbc_37_mat_print(rbc_37_mat m, uint32_t rows, uint32_t columns)
 * \brief Display an rbc_37_mat element.
 *
 * \param[out] m rbc_37_mat
 * \param[in] rows Row size of the rbc_37_mat
 * \param[in] columns Column size of the rbc_37_mat
 */
void rbc_37_mat_print(rbc_37_mat m, uint32_t rows, uint32_t columns) {
    printf("[\n");
    for(size_t i = 0 ; i < rows ; ++i) {
        printf("[\t");
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_37_elt_print(m[i][j]);
        }
        printf("\t]\n");
    }
    printf("]\n");
}
