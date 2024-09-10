/**
 * \file rbc_31_mat.c
 * \brief Implementation of rbc_31_mat.h
 */

#include "rbc_31_mat.h"




/**
 * \fn void rbc_31_mat_init(rbc_31_mat* m, uint32_t rows, uint32_t columns)
 * \brief This function allocates the memory for a rbc_31_mat.
 *
 * \param[out] m Pointer to the allocated rbc_31_mat
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat
 */
void rbc_31_mat_init(rbc_31_mat* m, uint32_t rows, uint32_t columns) {
    *m = calloc(rows, sizeof(rbc_31_vec *));
    (*m)[0] = calloc(rows * columns, sizeof(rbc_31_elt));
    for(size_t i = 0; i < rows; ++i) {
        (*m)[i] = (*m)[0] + (i * columns);
    }
    if (m == NULL) exit(EXIT_FAILURE);
}




/**
 * \fn void rbc_31_mat_clear(rbc_31_mat m)
 * \brief This function clears a rbc_31_mat element.
 *
 * \param[out] m rbc_31_mat
 */
void rbc_31_mat_clear(rbc_31_mat m) {
    free(m[0]);
    free(m);
}



/**
 * \fn void rbc_31_mat_set_zero(rbc_31_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite elements to zero.
 *
 * \param[out] m rbc_31_mat
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat
 */
void rbc_31_mat_set_zero(rbc_31_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_31_vec_set_zero(m[i], columns);
    }
}



/**
 * \fn void rbc_31_mat_set_random(seedexpander_shake* ctx, rbc_31_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite field elements with random values using NIST seed expander.
 *
 * \param[out] ctx Seed expander
 * \param[out] m rbc_31_mat
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat
 */
void rbc_31_mat_set_random(seedexpander_shake_t* ctx, rbc_31_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_31_vec_set_random(ctx, m[i], columns);
    }
}



/**
 * \fn void rbc_31_mat_vec_mul(rbc_31_vec o, const rbc_31_mat m, const rbc_31_vec v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a matrix of finite field elements by a vector.
 *
 * \param[out] o rbc_31_vec equal to \f$ m \times v \f$
 * \param[in] m rbc_31_mat
 * \param[in] v rbc_31_vec
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat, and Row size of the rbd_vec
 */
void rbc_31_mat_vec_mul(rbc_31_vec o, const rbc_31_mat m, const rbc_31_vec v, uint32_t rows, uint32_t columns) {
    rbc_31_elt tmp, acc;
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_31_elt_set_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_31_elt_mul(tmp, m[i][j], v[j]);
            rbc_31_elt_add(acc, acc, tmp);
        }
        rbc_31_elt_set(o[i], acc);
    }
}



/**
 * \fn void rbc_31_mat_to_string(uint8_t* str, const rbc_31_mat m, uint32_t rows, uint32_t columns)
 * \brief This function parses a matrix of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] m rbc_31_mat
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat
 */
void rbc_31_mat_to_string(uint8_t* str, const rbc_31_mat m, uint32_t rows, uint32_t columns) {
  rbc_31_vec t;
  rbc_31_vec_init(&t, rows * columns);
  for(size_t i = 0 ; i < rows ; i++) {
    for(size_t j = 0 ; j < columns ; j++) {
      rbc_31_elt_set(t[i * columns + j], m[i][j]);
    }
  }
  rbc_31_vec_to_string(str, t, rows * columns);
  rbc_31_vec_clear(t);
}



/**
 * \fn void rbc_31_mat_print(rbc_31_mat m, uint32_t rows, uint32_t columns)
 * \brief Display an rbc_31_mat element.
 *
 * \param[out] m rbc_31_mat
 * \param[in] rows Row size of the rbc_31_mat
 * \param[in] columns Column size of the rbc_31_mat
 */
void rbc_31_mat_print(rbc_31_mat m, uint32_t rows, uint32_t columns) {
    printf("[\n");
    for(size_t i = 0 ; i < rows ; ++i) {
        printf("[\t");
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_31_elt_print(m[i][j]);
        }
        printf("\t]\n");
    }
    printf("]\n");
}
