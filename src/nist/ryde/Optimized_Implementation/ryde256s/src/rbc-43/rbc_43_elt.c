/**
 * \file rbc_43_elt.c
 * \brief Implementation of rbc_43_elt.h
 */

#include "rbc_43.h"
#include "rbc_43_elt.h"

static uint8_t rbc_43_init_field = 0;
uint64_t RBC_43_SQR_LOOKUP_TABLE[256];



/**
 * \fn void rbc_43_field_init(void)
 * \brief This function initializes various constants used to perform finite field arithmetic.
 *
 */
void rbc_43_field_init(void) {
  uint8_t bit = 0;
  uint64_t mask = 0;

  if(rbc_43_init_field == 0) {
    memset(RBC_43_SQR_LOOKUP_TABLE, 0, 8 * 256);
    for(size_t i = 0 ; i < 256 ; ++i) {
      for(size_t j = 0 ; j < 8 ; ++j) {
        mask = 1 << j;
        bit = (mask & i) >> j;
        RBC_43_SQR_LOOKUP_TABLE[i] ^= bit << (2 * j);
      }
    }

    rbc_43_init_field = 1;
  }
}



/**
 * \fn void rbc_43_elt_set_zero(rbc_43_elt o)
 * \brief This function sets a finite field element to zero.
 *
 * \param[out] o rbc_43_elt
 */
void rbc_43_elt_set_zero(rbc_43_elt o) {
  o[0] = 0;
}



/**
 * \fn void rbc_43_elt_set_one(rbc_43_elt o)
 * \brief This function sets a finite field element to one.
 *
 * \param[out] o rbc_43_elt
 */
void rbc_43_elt_set_one(rbc_43_elt o) {
  o[0] = 1;
}



/**
 * \fn void rbc_43_elt_set(rbc_43_elt o, const rbc_43_elt e)
 * \brief This function copies a finite field element into another one.
 *
 * \param[out] o rbc_43_elt
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_set(rbc_43_elt o, const rbc_43_elt e) {
  o[0] = e[0];
}

/**
* \fn void rbc_43_elt_set_mask1(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2, uint32_t mask)
* \brief This function copies either e1 or e2 into o depending on the mask value
*
* \param[out] o rbc_43_elt
* \param[in] e1 rbc_43_elt
* \param[in] e2 rbc_43_elt_n* \param[in] mask 1 to copy e1 and 0 to copy e2
*/
void rbc_43_elt_set_mask1(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2, uint32_t mask) {
  for(uint8_t i = 0 ; i < RBC_43_ELT_SIZE ; i++) {
    o[i] = mask * e1[i] + (1 - mask) * e2[i];
  }
}

/**
 * \fn uint8_t rbc_43_elt_is_zero(const rbc_43_elt e)
 * \brief This function tests if a finite field element is equal to zero.
 * 
 * \param[in] e rbc_43_elt
 * \return 1 if <b>e</b> is equal to zero, 0 otherwise
 */
uint8_t rbc_43_elt_is_zero(const rbc_43_elt e) {
  return (e[0] == 0);
}

/**
 * \fn uint8_t rbc_43_elt_is_equal_to(const rbc_43_elt e1, const rbc_43_elt e2)
 * \brief This function tests if two finite field elements are equal.
 *
 * \param[in] e1 rbc_43_elt
 * \param[in] e2 rbc_43_elt
 * \return 1 if <b>e1</b> and <b>e2</b> are equal, 0 otherwise
 */
uint8_t rbc_43_elt_is_equal_to(const rbc_43_elt e1, const rbc_43_elt e2) {
  return (e1[0] == e2[0]);
}

/**
 * \fn int32_t rbc_43_elt_get_degree(const rbc_43_elt e)
 * \brief This function returns the degree of a finite field element.
 *
 * \param[in] e rbc_43_elt
 * \return Degree of <b>e</b> 
 */
int32_t rbc_43_elt_get_degree(const rbc_43_elt e) {
  int64_t index = 0, result = -1;
  int8_t mask = 0;

  __asm__ volatile("bsr %1,%0;" : "=r"(index) : "r"(e[0]));
  mask = (e[0] != 0);
  result = mask * index + (1 - mask) * result;
  return result;
}

/**
 * \fn uint8_t rbc_43_elt_get_coefficient(const rbc_43_elt e, uint32_t index)
 * \brief This function returns the coefficient of the polynomial <b>e</b> at a given index.
 *
 * \param[in] e rbc_43_elt
 * \param[in] index Index of the coefficient
 * \return Coefficient of <b>e</b> at the given index
 */
uint8_t rbc_43_elt_get_coefficient(const rbc_43_elt e, uint32_t index) {
  uint64_t w = 0;

  w |= -((0 ^ (index >> 6)) == 0) & e[0];

  return (w >> (index & 63)) & 1;
}

/**
 * \fn void rbc_43_elt_set_coefficient_vartime(rbc_43_elt o, uint32_t index, uint64_t bit)
 * \brief This function set a coefficient of the polynomial <b>e</b>.
 *
 * \param[in] e rbc_43_elt
 * \param[in] index Index of the coefficient
 * \param[in] bit Value of the coefficient
 */
void rbc_43_elt_set_coefficient_vartime(rbc_43_elt o, uint32_t index, uint8_t bit) {
  size_t position = index / 64;
  o[position] |= (uint64_t) bit << (index % 64);
}

/**
 * \fn rbc_43_elt_add(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2)
 * \brief This function adds two finite field elements.
 *
 * \param[out] o Sum of <b>e1</b> and <b>e2</b>
 * \param[in] e1 rbc_43_elt
 * \param[in] e2 rbc_43_elt
 */
void rbc_43_elt_add(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2) {
  o[0] = e1[0] ^ e2[0];
}


/**
 * \fn void rbc_43_elt_mul(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2)
 * \brief This function multiplies two finite field elements.
 *
 * \param[out] o Product of <b>e1</b> and <b>e2</b>
 * \param[in] e1 rbc_43_elt
 * \param[in] e2 rbc_43_elt
 */
void rbc_43_elt_mul(rbc_43_elt o, const rbc_43_elt e1, const rbc_43_elt e2) {
  rbc_43_elt_ur tmp;
  rbc_43_elt_ur_mul(tmp, e1, e2);
  rbc_43_elt_reduce(o, tmp);
}



/**
 * \fn void rbc_43_elt_sqr(rbc_43_elt o, const rbc_43_elt e)
 * \brief This function computes the square of a finite field element.
 *
 * \param[out] o rbc_43_elt equal to \f$ e^{2} \f$
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_sqr(rbc_43_elt o, const rbc_43_elt e) {
  /*
  if(rbc_43_init_field == 0) {
    printf("Call to rbc_43_elt_sqr with uninitialized field\n");
    exit(1);
  }
  */

  rbc_43_elt_ur tmp;
  rbc_43_elt_ur_sqr(tmp, e);
  rbc_43_elt_reduce(o, tmp);
}

/**
 * \fn void rbc_43_elt_reduce(rbc_43_elt o, const rbc_43_elt_ur e)
 * \brief This function reduces a finite field element.
 *
 * \param[out] o rbc_43_elt equal to $ e \pmod f $
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_reduce(rbc_43_elt o, const rbc_43_elt_ur e) {
  uint64_t tmp = (e[0] >> 43) ^ (e[1] << 21) ^ (e[1] >> 19) ^ (e[1] >> 18) ^ (e[1] >> 16);
  o[0] = e[0] ^ tmp ^ (tmp << 3) ^ (tmp << 4) ^ (tmp << 6);
  o[0] &= 0x7FFFFFFFFFF;
}

/**
 * \fn void rbc_43_elt_print(const rbc_43_elt e)
 * \brief This function displays a finite field element.
 *
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_print(const rbc_43_elt e) {
  printf("[");
  printf(" %16" PRIx64 , e[0]);
  printf(" ]");
}

/**
 * \fn void rbc_43_elt_ur_set(rbc_43_elt_ur o, const rbc_43_elt_ur e)
 * \brief This function copies an unreduced finite field element into another one.
 *
 * \param[out] o rbc_43_elt
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_ur_set(rbc_43_elt_ur o, const rbc_43_elt_ur e) {
  for(uint8_t i = 0 ; i < RBC_43_ELT_UR_SIZE ; i++) {
    o[i] = e[i];
  }
}

/**
 * \fn void rbc_43_elt_ur_set_zero(rbc_43_elt_ur o)
 * \brief This function sets an unreduced finite field element to zero.
 *
 * \param[out] o rbc_43_elt_ur
 */
void rbc_43_elt_ur_set_zero(rbc_43_elt_ur o) {
  for(uint8_t i = 0 ; i < RBC_43_ELT_UR_SIZE ; i++) {
    o[i] = 0;
  }
}

/**
 * \fn void rbc_43_elt_ur_mul(rbc_43_elt_ur o, const rbc_43_elt e1, const rbc_43_elt e2)
 * \brief This function computes the unreduced multiplication of two finite field elements.
 *
 * \param[out] o rbc_43_elt equal to \f$ e_1 \times e_2 $
 * \param[in] e1 rbc_43_elt
 * \param[in] e2 rbc_43_elt
 */
void rbc_43_elt_ur_mul(rbc_43_elt_ur o, const rbc_43_elt e1, const rbc_43_elt e2) {
  __m128i a = _mm_loadl_epi64((__m128i*) e1);
  __m128i b = _mm_loadl_epi64((__m128i*) e2);

  __m128i a0_b0 = _mm_clmulepi64_si128(a, b, 0x00);

  _mm_store_si128((__m128i*) o, a0_b0);
}

/**
 * \fn void rbc_43_elt_ur_sqr(rbc_43_elt o, const rbc_43_elt e)
 * \brief This function computes the unreduced square of a finite field element.
 *
 * \param[out] o rbc_43_elt_ur equal to $ e^{2} $
 * \param[in]  e rbc_43_elt
*/
void rbc_43_elt_ur_sqr(rbc_43_elt_ur o, const rbc_43_elt e) {
  __m128i tmp, e_l, e_h;

  tmp = _mm_loadl_epi64((__m128i*) e);
  e_l = _mm_and_si128(tmp, RBC_43_ELT_SQR_MASK_128);
  e_h = _mm_and_si128(_mm_srli_epi64(tmp, 4), RBC_43_ELT_SQR_MASK_128);

  e_l = _mm_shuffle_epi8(RBC_43_ELT_SQR_LOOKUP_TABLE_128, e_l);
  e_h = _mm_shuffle_epi8(RBC_43_ELT_SQR_LOOKUP_TABLE_128, e_h);

  tmp = _mm_unpacklo_epi8(e_l, e_h);
  _mm_store_si128((__m128i*) o, tmp);}




/**
 * \fn void rbc_43_elt_to_string(uint8_t* str, const rbc_43_elt e)
 * \brief This function parses a finite field element into a string.
 *
 * \param[out] str Output string
 * \param[in] e rbc_43_elt
 */
void rbc_43_elt_to_string(uint8_t* str, const rbc_43_elt e) {
  uint32_t bytes1 = RBC_43_FIELD_M / 8;
  uint32_t bytes2 = RBC_43_FIELD_M % 8;

  memset(str, 0, RBC_43_ELT_UINT8_SIZE);
  memcpy(str, e, bytes1);

  uint8_t k = 0;
  for(size_t j = 1 ; j <= bytes2 ; j++) {
    uint8_t bit = rbc_43_elt_get_coefficient(e, RBC_43_FIELD_M - j);
    *(str + bytes1) |= (bit << k % 8);
    k++;
    if(k % 8 == 0) bytes1++;
  }
}

