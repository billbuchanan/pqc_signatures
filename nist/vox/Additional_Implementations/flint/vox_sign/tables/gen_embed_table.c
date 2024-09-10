/******************************************************************************
 * Generate the translation bases to handle the QR-compression related
 * to the F_q -> F_q^c embedding
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Gilles Macario-Rat, Robin Larrieu
 *****************************************************************************/
#include "../vox_params.h"
#include <stdint.h>
#include "flint/flint.h"
#include "flint/fmpz.h"
#include "flint/fq_nmod.h"
#include "flint/nmod_poly.h"
#include "flint/nmod_mat.h"


#if VERBOSE>0
#define DEBUG_PRINTF      flint_printf
#define DEBUG_POLY(poly)  nmod_poly_print_pretty(poly, "X")
#define DEBUG_MAT(mat)    nmod_mat_print_pretty(mat)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_POLY(pol)
#define DEBUG_MAT(mat)
#endif

#ifdef DEF_POLY
static const uint16_t def_poly_coeffs[VOX_C+1] = DEF_POLY;
#endif


/*
 * Checks that a matrix is symmetric
 */
int nmod_mat_IsSymm(nmod_mat_t mat)
{
    int i, j, res = 1;
    int r = nmod_mat_nrows(mat);
    for (i = 1; i < r; i++) {
        for (j = 0; j < i; j++) {
            res = res && (nmod_mat_entry(mat, i, j) == nmod_mat_entry(mat, j, i));
        }
    }
    return res;
}


int main()
{
    int i, j, k, res, y;
    nmod_mat_t mat_companion, mat_mu, mat_tmp1, mat_tmp2;

    char str_file_name[100];
    FILE* fp;
    nmod_t Fq_ctx;
    nmod_mat_t base1[VOX_C+1]; /* Translation basis to extend operator T from F_q^c to F_q */
    nmod_mat_t base2[VOX_C+1]; /* Translation basis to extend quadratic equations from F_q^c to F_q */

    DEBUG_PRINTF("\n gen_embed_table start \n");
    flint_sprintf(str_file_name, "embed_%d_%d_.h", VOX_Q, VOX_C);
    fp = fopen(str_file_name, "w");
    if (fp == NULL) {
        DEBUG_PRINTF("\n gen_embed_table : fopen failed! \n");
        flint_throw(FLINT_ERROR, "gen_embed_table : fopen failed!\n");
        return -1;
    }

    nmod_init(&Fq_ctx, VOX_Q);
    for (i = 0; i < VOX_C + 1; i++) {
        nmod_mat_init(base1[i], VOX_C, VOX_C, VOX_Q);
        nmod_mat_init(base2[i], VOX_C, VOX_C, VOX_Q);
    }

#ifdef DEF_POLY
    nmod_poly_t def_poly;
    nmod_poly_init2(def_poly, VOX_Q, VOX_C+1);
    for (i = 0; i < VOX_C + 1; i++)
        nmod_poly_set_coeff_ui(def_poly, i, def_poly_coeffs[i]);
#else /* Use FLINTs default polynomial */
    fq_nmod_ctx_t ctx;
    fmpz_t prime;
    fmpz_init_set_si(prime, VOX_Q);
    fq_nmod_ctx_init(ctx, prime, VOX_C, "a");
    nmod_poly_struct const* def_poly = fq_nmod_ctx_modulus(ctx);
#endif
    DEBUG_PRINTF("\n gen_embed_table fq_nmod_poly_print_pretty \n");
    DEBUG_POLY(def_poly);
    DEBUG_PRINTF("\n");
    nmod_poly_t pinv, x, tr;
    nmod_poly_init(pinv, VOX_Q);
    nmod_poly_init(x, VOX_Q);
    nmod_poly_init(tr, VOX_Q);
    nmod_poly_reverse(pinv, def_poly, VOX_C+1);
    nmod_poly_inv_series(pinv, pinv, VOX_C+1);

    /* companion matrix */
    nmod_mat_init(mat_companion, VOX_C, VOX_C, VOX_Q);
    for (i = 0; i<VOX_C; i++) {
        y = nmod_poly_get_coeff_ui(def_poly, i);
        nmod_mat_entry(mat_companion, VOX_C - 1, i) = nmod_neg(y, Fq_ctx);
        if (i > 0) {
            nmod_mat_entry(mat_companion, i - 1, i) = 1;
        }
    }

    /* Compute mu with mu_(i,j) = Tr(X^(i+j) mod VOX_Q) */
    nmod_mat_init(mat_mu, VOX_C, VOX_C, VOX_Q);
    for (k = 0; k < 2*VOX_C - 1; k++) {
        nmod_poly_powmod_x_ui_preinv(x, k, def_poly, pinv);
        nmod_poly_set(tr, x);
        for (i = 1; i < VOX_C; i++) {
            nmod_poly_powmod_ui_binexp_preinv(x, x, VOX_Q, def_poly, pinv);
            nmod_poly_add(tr, tr, x);
        }
        for (i = 1; i < VOX_C; i++) {
            if (nmod_poly_get_coeff_ui(tr, i) != 0) {
                flint_throw(FLINT_ERROR, "gen_embed_table : trace computation failed!\n");
                return -1;
            }
        }
        y = nmod_poly_get_coeff_ui(tr, 0);
        for (i = 0; i <= k; i++) {
            if ((i < VOX_C) && (k-i < VOX_C))
                nmod_mat_entry(mat_mu, i, k-i) = y;
        }
    }
    DEBUG_PRINTF("\n gen_embed_table mat_mu =\n");
    DEBUG_MAT(mat_mu);
    DEBUG_PRINTF("\n gen_embed_table mat_companion =\n");
    DEBUG_MAT(mat_companion);

    /* mat_mu and (mat_companion * mat_mu) must all be symmetric */
    nmod_mat_init(mat_tmp1, VOX_C, VOX_C, VOX_Q);
    res = nmod_mat_IsSymm(mat_mu);
    DEBUG_PRINTF("\n gen_embed_table IsSymm(mat_mu) %d %s\n", res, res == 1 ? "OK" : "KO");

    if (res != 1) {
        flint_throw(FLINT_ERROR, "gen_embed_table : IsSymm mat_mu failed!\n");
        return -1;
    }

    nmod_mat_mul(mat_tmp1, mat_companion, mat_mu);
    res = nmod_mat_IsSymm(mat_tmp1);

    DEBUG_PRINTF("\n gen_embed_table IsSymm(mat_companion * mat_mu) %d %s\n", res, res == 1 ? "OK" : "KO");

    if (res != 1) {
        flint_throw(FLINT_ERROR, "gen_embed_table : IsSymm mat_companion * mat_mu failed!\n");
        return -1;
    }

    /*
     * base1 is computed from [ mat_companion^k for k<c ]
     * base2 is computed from [ mat_companion^k * mu for k<c ]
     */
    nmod_mat_init(mat_tmp2, VOX_C, VOX_C, VOX_Q);
    for (i = 0; i < VOX_C; i++) {
        nmod_mat_entry(mat_tmp1, i, i) = 1;
    }
    nmod_mat_set(mat_tmp2, mat_mu);

    for (j = 0; j < VOX_C; j++) {
        for (k = 0; k < VOX_C; k++) {
            nmod_mat_entry(base1[k], j, 0) = nmod_mat_entry(mat_tmp1, j, k);
            nmod_mat_entry(base2[k], j, 0) = nmod_mat_entry(mat_tmp2, j, k);
        }
    }

    for (i = 1; i < VOX_C; i++) {
        nmod_mat_mul(mat_tmp1, mat_companion, mat_tmp1);
        nmod_mat_mul(mat_tmp2, mat_companion, mat_tmp2);
        for (j = 0; j < VOX_C; j++) {
            for (k = 0; k < VOX_C; k++) {
                nmod_mat_entry(base1[k], j, i) = nmod_mat_entry(mat_tmp1, j, k);
                nmod_mat_entry(base2[k], j, i) = nmod_mat_entry(mat_tmp2, j, k);
            }
        }
    }
    /*
     * Add the matrix for the inverse conversion
     */
    res = nmod_mat_inv(base1[VOX_C], base1[0]);
    if (res != 1) {
        flint_throw(FLINT_ERROR, "gen_embed_table : nmod_mat_inv base1[0] failed!\n");
        return -1;
    }
    res = nmod_mat_inv(base2[VOX_C], base2[0]);
    if (res != 1) {
        flint_throw(FLINT_ERROR, "gen_embed_table : nmod_mat_inv base2[0] failed!\n");
        return -1;
    }

    /*
     * Save the result into a file to be included in vox_q2.c
     */
    flint_fprintf(fp, "/* Auto-generated using gen_embed_table.c */\n");
    flint_fprintf(fp, "#include <stdint.h>\n");
    flint_fprintf(fp, "\nconst uint16_t embed_table_base1[%d][%d][%d] =\n{\n", VOX_C+1, VOX_C, VOX_C);
    for (k = 0; k < VOX_C + 1; k++) {
        flint_fprintf(fp, "  {\n");
        for (i = 0; i < VOX_C; i++) {
            flint_fprintf(fp, "    {");
            for (j = 0; j < VOX_C; j++) {
                flint_fprintf(fp, " %4u,", (unsigned int) nmod_mat_entry(base1[k], i, j));
            }
            flint_fprintf(fp, " },\n");
        }
        flint_fprintf(fp, "  },\n");
    }
    flint_fprintf(fp, "};\n\n");

    flint_fprintf(fp, "\nconst uint16_t embed_table_base2[%d][%d][%d] =\n{\n", VOX_C+1, VOX_C, VOX_C);
    for (k = 0; k < VOX_C + 1; k++) {
        flint_fprintf(fp, "  {\n");
        for (i = 0; i < VOX_C; i++) {
            flint_fprintf(fp, "    {");
            for (j = 0; j < VOX_C; j++) {
                flint_fprintf(fp, " %4u,", (unsigned int) nmod_mat_entry(base2[k], i, j));
            }
            flint_fprintf(fp, " },\n");
        }
        flint_fprintf(fp, "  },\n");
    }
    flint_fprintf(fp, "};\n");

    fclose(fp);
    nmod_mat_clear(mat_companion);
    nmod_mat_clear(mat_mu);
    nmod_mat_clear(mat_tmp1);
    nmod_mat_clear(mat_tmp2);
    for (i = 0; i < VOX_C + 1; i++) {
        nmod_mat_clear(base1[i]);
        nmod_mat_clear(base2[i]);
    }
#ifdef DEF_POLY
    nmod_poly_clear(def_poly);
#else
    fmpz_clear(prime);
    fq_nmod_ctx_clear(ctx);
#endif
    DEBUG_PRINTF("\n gen_embed_table : return \n");
    return 0;
}

