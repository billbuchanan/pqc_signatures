/**
 * \file rbc_31_vspace.h
 * \brief Interface for subspaces of Fq^m
 */

#ifndef RBC_31_VSPACE_H
#define RBC_31_VSPACE_H

#include "rbc_31.h"

#include "seedexpander_shake.h"

void rbc_31_vspace_init(rbc_31_vspace* vs, uint32_t size);

void rbc_31_vspace_clear(rbc_31_vspace vs);

void rbc_31_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_31_vspace o, uint32_t size);

#endif

