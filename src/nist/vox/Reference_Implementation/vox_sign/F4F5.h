/******************************************************************************
 * Resolution of a quadratic system in T variables
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Jean-Charles Faugère, Robin Larrieu
 *****************************************************************************/
#ifndef VOX_F4F5_H
#define VOX_F4F5_H

#include "vox_params.h"

int Solve_MQ(Fq sol[VOX_T], const Fq eqns[VOX_T*((VOX_T+1)*(VOX_T+2))/2], const Fq hint);

#endif /* VOX_F4F5_H */
