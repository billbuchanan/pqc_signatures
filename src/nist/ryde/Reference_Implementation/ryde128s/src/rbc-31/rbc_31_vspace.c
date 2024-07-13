/**
 * \file rbc_31_vspace.c
 * \brief Implementation of rbc_31_vspace.h
 */

#include "rbc_31.h"
#include "rbc_31_vspace.h"
#include "rbc_31_vec.h"




/**
 * \fn void rbc_31_vspace_init(rbc_31_vspace* vs, uint32_t size)
 * \brief This function allocates the memory for a rbc_31_vspace.
 *
 * \param[out] vs Pointer to the allocated rbc_31_vspace
 * \param[in] size Size of the rbc_31_vspace
 */
void rbc_31_vspace_init(rbc_31_vspace* vs, uint32_t size) {
  rbc_31_vec_init(vs, size);
}




/**
 * \fn void rbc_31_vspace_clear(rbc_31_vspace vs)
 * \brief This functions clears the memory allocated to a rbc_31_vspace.
 *
 * \param[in] v rbc_31_vspace
 * \param[in] size Size of the rbc_31_vspace
 */
void rbc_31_vspace_clear(rbc_31_vspace vs) {
  rbc_31_vec_clear(vs);
}




/**
 * \fn void rbc_31_vspace_set_random_full_rank_with_one(seedexpander_shake* ctx, rbc_31_vspace o, uint32_t size) {
 * \brief This function sets a rbc_31_vspace with random values using a seed expander. The rbc_31_vspace returned by this function has full rank and contains one.
 *
 * \param[out] ctx Seed expander
 * \param[out] o rbc_31_vspace
 * \param[in] size Size of rbc_31_vspace
 */
void rbc_31_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_31_vspace o, uint32_t size) {
  rbc_31_vec_set_random_full_rank_with_one(ctx, o, size);
}

