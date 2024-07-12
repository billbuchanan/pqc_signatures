/**
 * \file rbc_43_mat.c
 * \brief Implementation of rbc_43_mat.h
 */

#include "rbc_43_mat.h"




/**
 * \fn void rbc_43_mat_init(rbc_43_mat* m, uint32_t rows, uint32_t columns)
 * \brief This function allocates the memory for a rbc_43_mat.
 *
 * \param[out] m Pointer to the allocated rbc_43_mat
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat
 */
void rbc_43_mat_init(rbc_43_mat* m, uint32_t rows, uint32_t columns) {
    *m = calloc(rows, sizeof(rbc_43_vec *));
    (*m)[0] = calloc(rows * columns, sizeof(rbc_43_elt));
    for(size_t i = 0; i < rows; ++i) {
        (*m)[i] = (*m)[0] + (i * columns);
    }
    if (m == NULL) exit(EXIT_FAILURE);
}




/**
 * \fn void rbc_43_mat_clear(rbc_43_mat m)
 * \brief This function clears a rbc_43_mat element.
 *
 * \param[out] m rbc_43_mat
 */
void rbc_43_mat_clear(rbc_43_mat m) {
    free(m[0]);
    free(m);
}



/**
 * \fn void rbc_43_mat_set_zero(rbc_43_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite elements to zero.
 *
 * \param[out] m rbc_43_mat
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat
 */
void rbc_43_mat_set_zero(rbc_43_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_43_vec_set_zero(m[i], columns);
    }
}



/**
 * \fn void rbc_43_mat_set_random(seedexpander_shake* ctx, rbc_43_mat m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite field elements with random values using NIST seed expander.
 *
 * \param[out] ctx Seed expander
 * \param[out] m rbc_43_mat
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat
 */
void rbc_43_mat_set_random(seedexpander_shake_t* ctx, rbc_43_mat m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_43_vec_set_random(ctx, m[i], columns);
    }
}



/**
 * \fn void rbc_43_mat_vec_mul(rbc_43_vec o, const rbc_43_mat m, const rbc_43_vec v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a matrix of finite field elements by a vector.
 *
 * \param[out] o rbc_43_vec equal to \f$ m \times v \f$
 * \param[in] m rbc_43_mat
 * \param[in] v rbc_43_vec
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat, and Row size of the rbd_vec
 */
void rbc_43_mat_vec_mul(rbc_43_vec o, const rbc_43_mat m, const rbc_43_vec v, uint32_t rows, uint32_t columns) {
    rbc_43_elt tmp, acc;
    for(size_t i = 0 ; i < rows ; ++i) {
        rbc_43_elt_set_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_43_elt_mul(tmp, m[i][j], v[j]);
            rbc_43_elt_add(acc, acc, tmp);
        }
        rbc_43_elt_set(o[i], acc);
    }
}



/**
 * \fn void rbc_43_mat_to_string(uint8_t* str, const rbc_43_mat m, uint32_t rows, uint32_t columns)
 * \brief This function parses a matrix of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] m rbc_43_mat
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat
 */
void rbc_43_mat_to_string(uint8_t* str, const rbc_43_mat m, uint32_t rows, uint32_t columns) {
  rbc_43_vec t;
  rbc_43_vec_init(&t, rows * columns);
  for(size_t i = 0 ; i < rows ; i++) {
    for(size_t j = 0 ; j < columns ; j++) {
      rbc_43_elt_set(t[i * columns + j], m[i][j]);
    }
  }
  rbc_43_vec_to_string(str, t, rows * columns);
  rbc_43_vec_clear(t);
}



/**
 * \fn void rbc_43_mat_print(rbc_43_mat m, uint32_t rows, uint32_t columns)
 * \brief Display an rbc_43_mat element.
 *
 * \param[out] m rbc_43_mat
 * \param[in] rows Row size of the rbc_43_mat
 * \param[in] columns Column size of the rbc_43_mat
 */
void rbc_43_mat_print(rbc_43_mat m, uint32_t rows, uint32_t columns) {
    printf("[\n");
    for(size_t i = 0 ; i < rows ; ++i) {
        printf("[\t");
        for(size_t j = 0 ; j < columns ; ++j) {
            rbc_43_elt_print(m[i][j]);
        }
        printf("\t]\n");
    }
    printf("]\n");
}
