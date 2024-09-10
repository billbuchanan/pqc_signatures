/**
 * \file rbc_37_vspace.h
 * \brief Interface for subspaces of Fq^m
 */

#ifndef RBC_37_VSPACE_H
#define RBC_37_VSPACE_H

#include "rbc_37.h"

#include "seedexpander_shake.h"

void rbc_37_vspace_init(rbc_37_vspace* vs, uint32_t size);

void rbc_37_vspace_clear(rbc_37_vspace vs);

void rbc_37_vspace_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_37_vspace o, uint32_t size);

#endif

