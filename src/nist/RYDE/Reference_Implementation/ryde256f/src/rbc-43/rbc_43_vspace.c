/**
 * \file rbc_43_vspace.c
 * \brief Implementation of rbc_43_vspace.h
 */

#include "rbc_43.h"
#include "rbc_43_vspace.h"
#include "rbc_43_vec.h"




/**
 * \fn void rbc_43_vspace_init(rbc_43_vspace* vs, uint32_t size)
 * \brief This function allocates the memory for a rbc_43_vspace.
 *
 * \param[out] vs Pointer to the allocated rbc_43_vspace
 * \param[in] size Size of the rbc_43_vspace
 */
void rbc_43_vspace_init(rbc_43_vspace* vs, uint32_t size) {
  rbc_43_vec_init(vs, size);
}




/**
 * \fn void rbc_43_vspace_clear(rbc_43_vspace vs)
 * \brief This functions clears the memory allocated to a rbc_43_vspace.
 *
 * \param[in] v rbc_43_vspace
 * \param[in] size Size of the rbc_43_vspace
 */
void rbc_43_vspace_clear(rbc_43_vspace vs) {
  rbc_43_vec_clear(vs);
}




/**
 * \fn void rbc_43_vspace_set_random_full_rank_with_one(seedexpander_shake* ctx, rbc_43_vspace o, uint32_t size) {
 * \brief This function sets a rbc_43_vspace with random values using a seed expander. The rbc_43_vspace returned by this function has full rank and contains one.
 *
 * \param[out] ctx Seed expander
 * \param[out] o rbc_43_vspace
 * \param[in] size Size of rbc_43_vspace
 */
void rbc_43_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_43_vspace o, uint32_t size) {
  rbc_43_vec_set_random_full_rank_with_one(ctx, o, size);
}

