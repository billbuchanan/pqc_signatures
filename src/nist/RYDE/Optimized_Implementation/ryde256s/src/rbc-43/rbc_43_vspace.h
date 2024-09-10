/**
 * \file rbc_43_vspace.h
 * \brief Interface for subspaces of Fq^m
 */

#ifndef RBC_43_VSPACE_H
#define RBC_43_VSPACE_H

#include "rbc_43.h"

#include "seedexpander_shake.h"

void rbc_43_vspace_init(rbc_43_vspace* vs, uint32_t size);

void rbc_43_vspace_clear(rbc_43_vspace vs);

void rbc_43_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_43_vspace o, uint32_t size);

#endif

