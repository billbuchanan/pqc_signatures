/******************************************************************************
 * High level demo of the VOX cryptosystem
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Gilles Macario-Rat
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../vox.h"
#include "../rng.h"

/*
 * Generates a key pair, a target vector, signs and verifies
 */
void Vox_test()
{
    int i;
    int res = 0;
    int nb_res;
    int nb_sign = 100;
    int nb_gen = 10;
    nmod_mat_t S, T, Pub[VOX_O], Sec[VOX_O], Signature, Message;
    clock_t t;
    unsigned char hpk[VOX_HPK_BYTES];
    unsigned char seed_PK[VOX_SEED_BYTES];
    unsigned char seed_SK[VOX_SEED_BYTES];
    unsigned char msg[VOX_SEED_BYTES];
    unsigned char seed_V[VOX_SEED_BYTES];

    for (i = 0; i < VOX_O; i++) {
        nmod_mat_init(Pub[i], VOX_N, VOX_N, VOX_Q);
        nmod_mat_init(Sec[i], VOX_N, VOX_N, VOX_Q);
    }
    nmod_mat_init(T, VOX_N, VOX_N, VOX_Q);
    nmod_mat_init(S, VOX_O, VOX_O, VOX_Q);
    nmod_mat_init(Signature, 1, VOX_N, VOX_Q);
    nmod_mat_init(Message, 1, VOX_O, VOX_Q);

#if VERBOSE>0
    flint_printf("nlimbs = %d\n", _nmod_vec_dot_bound_limbs(VOX_N, mod_p));
#endif
    flint_printf("\ntime GenVoxKeys %d gen start \n", nb_gen);
    randombytes(seed_PK, VOX_SEED_BYTES);
    randombytes(seed_SK, VOX_SEED_BYTES);
    randombytes(hpk, VOX_HPK_BYTES); /* use a fake hash to generate the target vector since we do not care for this test */
    t = clock();
    for (i = 0; i < nb_gen; i++) {
        GenVoxKeys(Pub, Sec, S, T, seed_PK, seed_SK);
    }
    t = clock() - t;
    flint_printf("\ntime GenVoxKeys %d gen : %f\n", nb_gen,  1.0 * t / CLOCKS_PER_SEC);

    res = 1;
    nb_res = 0;

    flint_printf("\n Vox_test Sign\n");
    t = clock();
    for (i = 0; i < nb_sign; i++) {
        randombytes(msg, VOX_SEED_BYTES);
        randombytes(seed_V, VOX_SEED_BYTES);
        VOX_GenM(Message, hpk, msg, VOX_SEED_BYTES);
        Sign(Signature, Message, Sec, S, T, seed_V);
        res = Verify(Signature, Message, Pub);
        if (res == 1) {
            nb_res++;
        }
    }
    t = clock() - t;
    flint_printf("\n Vox_test Verify succes %d %s\n", nb_res, nb_res == nb_sign ? "OK" : "!!! KO !!!");
    flint_printf("\ntime verify %d sign : %f\n", nb_sign, 1.0 * t / CLOCKS_PER_SEC);
#if VERBOSE>0
    display_counters();
#endif /*  VERBOSE>0 */

    for (i = 0; i < VOX_O; i++) {
        nmod_mat_clear(Pub[i]);
        nmod_mat_clear(Sec[i]);
    }
    nmod_mat_clear(T);
    nmod_mat_clear(S);
    nmod_mat_clear(Signature);
    nmod_mat_clear(Message);
    return;
}

int main(int argc, char* argv[])
{
    int i;
    unsigned char   seed_material[48];

    flint_printf("Parameter set : " VOX_NAME "\n");
    for (i = 0; i < 48; i++) {
        seed_material[i] = i;
    }
    randombytes_init(seed_material, NULL, 0);

    InitContext();

    Vox_test();

    ClearContext();
    return EXIT_SUCCESS;
}


