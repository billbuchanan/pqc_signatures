/**
 * \file rbc_37_vec.c
 * \brief Implementation of rbc_37_vec.h
 */

#include "rbc_37_vec.h"




/**
 * \fn void rbc_37_vec_init(rbc_37_vec* v, uint32_t size)
 * \brief This function initiates a rbc_37_vec element.
 *
 * \param[out] v Pointer to a rbc_37_vec element
 * \param[in] size Size of the vector
 */
void rbc_37_vec_init(rbc_37_vec* v, uint32_t size) {
  *v = calloc(size, sizeof(rbc_37_elt));
  if(v == NULL) exit(EXIT_FAILURE);
}




/**
 * \fn void rbc_37_vec_clear(rbc_37_vec v)
 * \brief This function clears a rbc_37_vec element.
 *
 * \param[out] v rbc_37_vec
 */
void rbc_37_vec_clear(rbc_37_vec v) {
  free(v);
}




/**
 * \fn void rbc_37_vec_set_zero(rbc_37_vec v, uint32_t size)
 * \brief This function sets a vector of finite elements to zero.
 *
 * \param[out] v rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_set_zero(rbc_37_vec v, uint32_t size) {
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_set_zero(v[i]);
  }
}




/**
 * \fn void rbc_37_vec_set(rbc_37_vec o, const rbc_37_vec v, uint32_t size)
 * \brief This function copies a vector of finite field elements to another one.
 *
 * \param[out] o rbc_37_vec
 * \param[in] v rbc_37_vec
 * \param[in] size Size of the vectors
 */
void rbc_37_vec_set(rbc_37_vec o, const rbc_37_vec v, uint32_t size) {
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_set(o[i], v[i]);
  }
}




/**
 * \fn void rbc_37_vec_set_random(seedexpander_shake_t*, rbc_37_vec v, uint32_t size)
 * \brief This function sets a vector of finite field elements with random values using NIST seed expander.
 *
 * \param[out] ctx Seed expander
 * \param[out] v rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_set_random(seedexpander_shake_t* ctx, rbc_37_vec v, uint32_t size) {
  uint32_t bytes = (RBC_37_FIELD_M % 8 == 0) ? RBC_37_FIELD_M / 8 : RBC_37_FIELD_M / 8 + 1;
  uint8_t random[size * bytes];
  uint8_t mask = (1 << RBC_37_FIELD_M % 8) - 1;

  rbc_37_vec_set_zero(v, size);
  seedexpander_shake_get_bytes(ctx, random, size * bytes);

  for(size_t i = 0 ; i < size ; ++i) {
    random[(i + 1) * bytes - 1] &= mask;
    memcpy(v[i], random + i * bytes, bytes);
  }
}




/**
 * \fn void rbc_37_vec_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_37_vec o, uint32_t size) {
 * \brief This function sets a vector with random values using a seed expander. The vector returned by this function has full rank and contains one as the last coordinate.
 *
 * \param[out] ctx Seed expander
 * \param[out] o rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_37_vec o, uint32_t size) {
  int32_t rank_max = RBC_37_FIELD_M < size ? RBC_37_FIELD_M : size;
  int32_t rank = -1;

  while(rank != rank_max) {
    rbc_37_vec_set_random(ctx, o, size - 1);
    rbc_37_elt_set_one(o[size - 1]);
    rank = rbc_37_vec_get_rank(o, size);
  }
}




/**
 * \fn void rbc_37_vec_set_random_from_support(seedexpander_shake_t* o, rbc_37_vec o, uint32_t size, const rbc_37_vec support, uint32_t support_size)
 * \brief This function sets a vector with random values using the NIST seed expander. The support of the vector returned by this function is included in the one given in input.
 *
 * \param[out] ctx Seed
 * \param[out] o rbc_37_vec
 * \param[in] size Size of <b>o</b>
 * \param[in] support Support of <b>v</b>
 * \param[in] rank Size of the support
 * \param[in] copy_flag If not 0, the support is copied into random coordinates of the resulting vector
 */
void rbc_37_vec_set_random_from_support(seedexpander_shake_t* ctx, rbc_37_vec o, uint32_t size, const rbc_37_vec support, uint32_t support_size, uint8_t copy_flag) {
  rbc_37_vec_set_zero(o, size);

  uint32_t i=0;
  uint32_t j=0;

  if(copy_flag) {
    uint32_t random1_size = 2 * support_size;
    uint32_t random2_size = (support_size * (size - support_size)) / 8 + 1;
    uint8_t random1[random1_size];
    uint8_t random2[random2_size];

    // Copy the support vector in support_size random positions of o
    seedexpander_shake_get_bytes(ctx, random1, random1_size);

    uint32_t position;
    while(i != support_size) {
      position = random1[j];

      // Check that the position is not already taken
      if(position < size * (256 / size) && rbc_37_elt_is_zero(o[position % size])) {
        rbc_37_elt_set(o[position % size], support[i]);
        i++;
      }

      // Get more randomness if necessary
      j++;
      if(j % random1_size == 0 && i != support_size) {
        seedexpander_shake_get_bytes(ctx, random1, random1_size);
        j = 0;
      }
    }

    // Set all the remaining coordinates with random linear combinations of the support coordinates
    seedexpander_shake_get_bytes(ctx, random2, random2_size);

    uint32_t k = 0;
    uint32_t l = 0;
    for(i = 0 ; i < size ; ++i) {
      if(rbc_37_elt_is_zero(o[i])) {

        for(j = 0 ; j < support_size ; ++j) {
          if(random2[k] & 0x1) {
            rbc_37_elt_add(o[i], support[j], o[i]);
          }

          random2[k] = random2[k] >> 1;
          l++;
          if(l == 8) {
            l = 0;
            k++;
          }

        }
      }
    }
  }
  else {
    // Set all the coordinates with random linear combinations of the support coordinates
    uint32_t random_size = support_size * size / 8 + 1;
    unsigned char random[random_size];
    seedexpander_shake_get_bytes(ctx, random, random_size);

    uint32_t k = 0;
    uint32_t l = 0;

    for(i=0 ; i<size ; i++) {
      for(j=0 ; j<support_size ; j++) {
        if(random[k] & 0x01) {
          rbc_37_elt_add(o[i], o[i], support[j]);
        }

        random[k] = random[k] >> 1;
        l++;
        if(l == 8) {
          l=0;
          k++;
        }
      }
    }
  }
}

/**
 * \fn uint32_t rbc_37_vec_gauss(rbc_37_vec v, uint32_t size, rbc_37_vec *other_matrices, uint32_t nMatrices)
 * \brief This function transform a vector of finite field elements to its row echelon form and returns its rank.
 *
 * Replicates linear operations on the nMatrices matrices indexed by other_matrices.
 *
 * \param[in] v rbc_37_vec
 * \param[in] size Size of the vector
 * \param[in] reduced_flag If set, the function computes the reduced row echelon form
 * \param[in] other_matrices Pointer to other matrices to replicate the operations on
 * \param[in] nMatrices Number of other matrices
 *
 * \return Rank of the vector <b>v</b>
 */
uint32_t rbc_37_vec_gauss(rbc_37_vec v, uint32_t size, uint8_t reduced_flag, rbc_37_vec *other_matrices, uint32_t nMatrices) {
  uint32_t dimension = 0;
  rbc_37_elt tmp, zero;
  uint32_t mask;
  rbc_37_elt_set_zero(zero);

  //For each column
  for(uint32_t p = 0 ; p < size ; p++) {
    rbc_37_elt acc;
    rbc_37_elt_set(acc, v[p]);
    for(uint32_t i=p+1 ; i<size ; i++) {
      for(uint32_t j=0 ; j<RBC_37_ELT_DATA_SIZE ; j++) {
        acc[j] |= v[i][j];
      }
    }

    int column = rbc_37_elt_get_degree(acc);
    column += (column < 0);

    dimension += (!rbc_37_elt_is_zero(acc));

    //For each line below
    for(uint32_t i=p+1 ; i < size ; i++) {
        mask = rbc_37_elt_get_coefficient(v[p], column);
        rbc_37_elt_set_mask1(tmp, zero, v[i], mask);
        rbc_37_elt_add(v[p], v[p], tmp);

        for(uint32_t k=0 ; k<nMatrices ; k++) {
          rbc_37_elt_set_mask1(tmp, zero, other_matrices[k][i], mask);
          rbc_37_elt_add(other_matrices[k][p], other_matrices[k][p], tmp);
        }
    }

    //For each other line
    if(reduced_flag) {
      for(uint32_t i=0 ; i < p ; i++) {
        mask = rbc_37_elt_get_coefficient(v[i], column);
        rbc_37_elt_set_mask1(tmp, v[p], zero, mask);
        rbc_37_elt_add(v[i], v[i], tmp);

        for(uint32_t k=0 ; k<nMatrices ; k++) {
          rbc_37_elt_set_mask1(tmp, other_matrices[k][p], zero, mask);
          rbc_37_elt_add(other_matrices[k][i], other_matrices[k][i], tmp);
        }
      }
    }

    for(uint32_t i=p+1 ; i < size ; i++) {
      mask = rbc_37_elt_get_coefficient(v[i], column);
      rbc_37_elt_set_mask1(tmp, v[p], zero, mask);
      rbc_37_elt_add(v[i], v[i], tmp);

      for(uint32_t k=0 ; k<nMatrices ; k++) {
        rbc_37_elt_set_mask1(tmp, other_matrices[k][p], zero, mask);
        rbc_37_elt_add(other_matrices[k][i], other_matrices[k][i], tmp);
      }
    }
  }

  return dimension;
}
/**
 * \fn uint32_t rbc_37_vec_get_rank(const rbc_37_vec v, uint32_t size)
 * \brief This function computes the rank of a vector of finite field elements.
 *
 * \param[in] v rbc_37_vec
 * \param[in] size Size of the vector
 *
 * \return Rank of the vector <b>v</b>
 */
uint32_t rbc_37_vec_get_rank(const rbc_37_vec v, uint32_t size) {
  rbc_37_vec copy;
  uint32_t dimension;

  rbc_37_vec_init(&copy, size);
  rbc_37_vec_set(copy, v, size);
  dimension = rbc_37_vec_gauss(copy, size, 0, NULL, 0);
  rbc_37_vec_clear(copy);

  return dimension;
}



/**
 * \fn void rbc_37_vec_add(rbc_37_vec o, const rbc_37_vec v1, const rbc_37_vec v2, uint32_t size)
 * \brief This function adds two vectors of finite field elements.
 *
 * \param[out] o Sum of <b>v1</b> and <b>v2</b>
 * \param[in] v1 rbc_37_vec
 * \param[in] v2 rbc_37_vec
 * \param[in] size Size of the vectors
 */
void rbc_37_vec_add(rbc_37_vec o, const rbc_37_vec v1, const rbc_37_vec v2, uint32_t size) {
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_add(o[i], v1[i], v2[i]);
  }
}




/**
 * \fn void rbc_37_vec_inner_product(rbc_37_elt o, const rbc_37_vec v1, const rbc_37_vec v2, uint32_t size)
 * \brief This function computes the inner product of two vectors over finite field elements.
 *
 * \param[out] o inner product of <b>v1</b> and <b>v2</b>
 * \param[in] v1 rbc_37_vec
 * \param[in] v2 rbc_37_vec
 * \param[in] size Size of the vectors
 */
void rbc_37_vec_inner_product(rbc_37_elt o, const rbc_37_vec v1, const rbc_37_vec v2, uint32_t size) {
  rbc_37_elt t;
  rbc_37_elt_set_zero(o);
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_mul(t, v1[i], v2[i]);
    rbc_37_elt_add(o, o, t);
  }
  rbc_37_elt_set_zero(t);
}




/**
 * \fn void rbc_37_vec_scalar_mul(rbc_37_vec o, const rbc_37_vec v, const rbc_37_elt e, uint32_t size)
 * \brief This functions multiplies a vector of finite field elements by a scalar.
 *
 * \param[out] o rbc_37_vec equal to \f$ e \times v \f$
 * \param[in] v rbc_37_vec
 * \param[in] e Finite field element
 * \param[in] size Size of the vector
 */
void rbc_37_vec_scalar_mul(rbc_37_vec o, const rbc_37_vec v, const rbc_37_elt e, uint32_t size) {
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_mul(o[i], v[i], e);
  }
}




/**
 * \fn void rbc_37_vec_to_string(uint8_t* str, const rbc_37_vec v, uint32_t size)
 * \brief This function parses a vector of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] v rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_to_string(uint8_t* str, const rbc_37_vec v, uint32_t size) {
  uint32_t bytes1 = RBC_37_FIELD_M / 8;
  uint32_t bytes2 = RBC_37_FIELD_M % 8;
  uint32_t index = bytes1 * size;
  uint32_t str_size = ((size * RBC_37_FIELD_M) % 8 == 0) ? (size * RBC_37_FIELD_M) / 8 : (size * RBC_37_FIELD_M) / 8 + 1;

  memset(str, 0, str_size);

  for(size_t i = 0 ; i < size ; i++) {
    memcpy(str + i * bytes1, v[i], bytes1);
  }

  uint8_t k = 0;
  for(size_t i = 0 ; i < size ; i++) {
    for(size_t j = 1 ; j <= bytes2 ; j++) {
      uint8_t bit = rbc_37_elt_get_coefficient(v[i], RBC_37_FIELD_M - j);
      *(str + index) |= (bit << k % 8);
      k++;
      if(k % 8 == 0) index++;
    }
  }
}




/**
 * \fn void rbc_37_vec_from_string(rbc_37_vec v, uint32_t size, const uint8_t* str)
 * \brief This function parses a string into a vector of finite field elements.
 *
 * \param[out] v rbc_37_vec
 * \param[in] size Size of the vector
 * \param[in] str String to parse
 */
void rbc_37_vec_from_string(rbc_37_vec v, uint32_t size, const uint8_t* str) {
  uint32_t bytes1 = RBC_37_FIELD_M / 8;
  uint32_t bytes2 = RBC_37_FIELD_M % 8;
  uint32_t index = bytes1 * size;

  rbc_37_vec_set_zero(v, size);

  for(size_t i = 0 ; i < size ; i++) {
    memcpy(v[i], str + i * bytes1, bytes1);
  }

  uint8_t k = 0;
  for(size_t i = 0 ; i < size ; i++) {
    for(size_t j = 1 ; j <= bytes2 ; j++) {
      uint8_t bit = (str[index] >> k % 8) & 0x01;
      rbc_37_elt_set_coefficient_vartime(v[i], RBC_37_FIELD_M - j, bit);
      k++;
      if(k % 8 == 0) index++;
    }
  }
}




/**
 * \fn void rbc_37_vec_from_bytes(rbc_37_vec o, uint32_t size, const uint8_t *random)
 * \brief This function sets a vector of finite field elements from random bytes.
 *
 * \param[out] v rbc_37_vec
 * \param[in] size Size of the vector
 * \param[in] random String containing random bytes
 */
void rbc_37_vec_from_bytes(rbc_37_vec o, uint32_t size, uint8_t *random) {
  uint8_t mask = (1 << RBC_37_FIELD_M % 8) - 1;
  if ((RBC_37_FIELD_M % 8) == 0) { mask = 0xff; }

  for(size_t i = 0 ; i < size ; ++i) {
    random[(i + 1) * RBC_37_ELT_UINT8_SIZE - 1] &= mask;
    memcpy(o[i], &random[i * RBC_37_ELT_UINT8_SIZE], RBC_37_ELT_UINT8_SIZE);
  }
}




/**
 * \fn void rbc_37_vec_to_bytes( uint8_t *o, const rbc_37_vec v, uint32_t size)
 * \brief This function sets a vector of finite field elements from random bytes.
 *
 * \param[out] o String containing v
 * \param[in] v rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_to_bytes( uint8_t *o, const rbc_37_vec v, uint32_t size) {
  for(size_t i = 0 ; i < size ; ++i) {
    memcpy(&o[i * RBC_37_ELT_UINT8_SIZE], v[i], RBC_37_ELT_UINT8_SIZE);
  }
}




/**
 * \fn void rbc_37_vec_print(rbc_37_vec v, uint32_t size)
 * \brief Display an rbc_37_vec element.
 *
 * \param[out] v rbc_37_vec
 * \param[in] size Size of the vector
 */
void rbc_37_vec_print(rbc_37_vec v, uint32_t size) {
  printf("[\n");
  for(size_t i = 0 ; i < size ; ++i) {
    rbc_37_elt_print(v[i]);
    printf("\n");
  }
  printf("]\n");
}

