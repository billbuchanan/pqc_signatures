/******************************************************************************
 * FGLM algorithm to change monomial ordering
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Jean-Charles Faugère, Robin Larrieu
 *****************************************************************************/
#ifndef VOX_FGLM_H
#define VOX_FGLM_H

#include "vox_params.h"

uint32_t W_FGLM_new(int32_t n, const int32_t* sparse, const int32_t* nosp,
                    int32_t nvars, const int32_t* i_e, Fq** cfs, const Fq hint);

#endif /* VOX_FGLM_H */
