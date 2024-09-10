/**
 * \file rbc_37_vspace.c
 * \brief Implementation of rbc_37_vspace.h
 */

#include "rbc_37.h"
#include "rbc_37_vspace.h"
#include "rbc_37_vec.h"




/**
 * \fn void rbc_37_vspace_init(rbc_37_vspace* vs, uint32_t size)
 * \brief This function allocates the memory for a rbc_37_vspace.
 *
 * \param[out] vs Pointer to the allocated rbc_37_vspace
 * \param[in] size Size of the rbc_37_vspace
 */
void rbc_37_vspace_init(rbc_37_vspace* vs, uint32_t size) {
  rbc_37_vec_init(vs, size);
}




/**
 * \fn void rbc_37_vspace_clear(rbc_37_vspace vs)
 * \brief This functions clears the memory allocated to a rbc_37_vspace.
 *
 * \param[in] v rbc_37_vspace
 * \param[in] size Size of the rbc_37_vspace
 */
void rbc_37_vspace_clear(rbc_37_vspace vs) {
  rbc_37_vec_clear(vs);
}




/**
 * \fn void rbc_37_vspace_set_random_full_rank_with_one(seedexpander_shake* ctx, rbc_37_vspace o, uint32_t size) {
 * \brief This function sets a rbc_37_vspace with random values using a seed expander. The rbc_37_vspace returned by this function has full rank and contains one.
 *
 * \param[out] ctx Seed expander
 * \param[out] o rbc_37_vspace
 * \param[in] size Size of rbc_37_vspace
 */
void rbc_37_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_37_vspace o, uint32_t size) {
  rbc_37_vec_set_random_full_rank_with_one(ctx, o, size);
}

