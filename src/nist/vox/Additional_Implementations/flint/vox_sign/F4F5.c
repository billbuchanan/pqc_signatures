/******************************************************************************
 * Solver for a quadratic system (t equations in t variables)
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Gilles Macario-Rat
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "vox.h"

enum TAGS {
END,
INIT_B,
COPY_A,
COPY_A_B,
PIVOT,
RREF,
INIT_D,
COPY_B_D,
COPY_A_D,
END_LOOP
};


#define prefix tables/F4F5_
#define suffix _.h

#ifdef CAT
#undef CAT
#endif

#define STRINGIZE_AUX(a) #a
#define STRINGIZE(a) STRINGIZE_AUX(a)
#define CAT_AUX(a, b) a##b
#define CAT(a, b) CAT_AUX(a, b)
#define MY_MACRO_AUX(name, subname) STRINGIZE(CAT(prefix, CAT (name,CAT( _,CAT(subname, suffix)))))
#define MY_MACRO(name,subname) MY_MACRO_AUX(name, subname)

#include MY_MACRO(VOX_T,1)
#include MY_MACRO(VOX_T,2)

static void do_pivot(nmod_mat_t *pB, short int* pTab1, short int* pTab2,
                     short int j, short int s1, short int s2, const nmod_t mod)
{
    for (slong i = 0; i < s1; i++) {
        const unsigned long x = nmod_neg(nmod_mat_entry(*pB, pTab1[i], j), mod);
        for (slong k = 0; k < s2; k++)
            NMOD_ADDMUL(nmod_mat_entry(*pB, pTab1[i], pTab2[k]),
                        x, nmod_mat_entry(*pB, j, pTab2[k]), mod);
    }
}


static int find_a_root(mp_limb_t *sol, nmod_poly_t poly, mp_limb_t hint)
{
  nmod_poly_factor_t res;
  nmod_poly_factor_init(res);
  nmod_poly_roots(res, poly, 0);

  int nbsol=res->num;
  if (nbsol == 0) {
    nmod_poly_factor_clear(res);
    return 0;
  }

  mp_limb_t root = hint + VOX_Q;
  for (int i=0; i<nbsol; i++) {
    nmod_poly_struct *fact = &(res->p[i]);
    assert(nmod_poly_degree(fact) == 1);
    mp_limb_t new_root = nmod_neg(nmod_poly_get_coeff_ui(fact, 0), poly->mod);
    if (new_root < hint)
      new_root += VOX_Q;
    if (new_root < root)
      root = new_root;
  }
  if (root >= VOX_Q)
    root -= VOX_Q;
  *sol = root;
  nmod_poly_factor_clear(res);
  return 1;
}


int verify_mq(nmod_mat_t A, nmod_mat_t B)
{
    int i,j,k,l,m;
    mp_limb_t x;
    int res = 0;
    int res_sol;
    for (m=0; m<B->c; m++)
    {
        res_sol = 0;
        for (k=0; k<A->r; k++)
        {
            x=0;
            l=0;
            for (i=0; i<N; i++)
            {
                for (j=0; j<=i; j++)
                {
                    x = nmod_addmul ( x, nmod_mul ( nmod_mat_entry(A, k, l++) ,
                        nmod_mat_entry(B, i, m), A->mod) ,
                        nmod_mat_entry(B, j, m), A->mod);
                }
            }
            for (i=0; i<N; i++)
            {
                x = nmod_addmul ( x, nmod_mat_entry(A, k, l++) ,
                    nmod_mat_entry(B, i, m), A->mod);
            }
            x = nmod_add(x, nmod_mat_entry(A, k, l++), A->mod);
            if (x == 0)
            {
                res_sol++;
            }
        }
        if (res_sol == A->r)
        {
            res++;
        }
    }
    return res;
}


int Solve_MQ(nmod_mat_t sol, nmod_mat_t System, const int full, mp_limb_t hint)
{
    int singular_system = 0, correct, sol_ok = 0;
    short int *pTable, *pTab1, *pTab2;
    short int Tag, i, j, k, l, s1, s2;
    nmod_mat_t A, B, C, D, E, CC, *pA, *pB, *pC, *pD;
    nmod_t mod;


    nmod_mat_init_set(A, System);
    mod = System->mod;

    nmod_mat_rref(A);

    for (i = 0; i < N; i++)
    {
        singular_system = singular_system || (nmod_mat_entry(A, i, i) != 1);
    }

    if (singular_system == 1)
    {
        nmod_mat_clear(A);
        goto _end;
    }

    pA = &A;
    pB = &B;
    pC = &C;
    pD = &D;


    pTable = Table;
    s1 = 0;
    s2 = 0;
    pTab1 = Table;
    pTab2 = Table;

    /*
     * Compute Grevlex Gröbner basis using F4
     */
    do {
        Tag = *pTable;
        pTable++;
        switch (Tag)
        {
        case INIT_B:
            i = *pTable++;
            j = *pTable++;
            nmod_mat_init(*pB, i, j, VOX_Q);
            break;
        case COPY_A:
            i = *pTable++;
            s1 = *pTable++;
            pTab1 = pTable;
            pTable += s1;
            break;
        case COPY_A_B:
            j = *pTable++;
            s2 = *pTable++;
            pTab2 = pTable;
            pTable += s2;
            assert(s1 == s2);
            for (k = 0; k < s1; k++) {
                nmod_mat_entry(*pB, j, pTab2[k]) = nmod_mat_entry(*pA, i, pTab1[k]);
            }
            break;
        case PIVOT:
            j = *pTable++;
            s1 = *pTable++;
            pTab1 = pTable; /* row indices */
            pTable += s1;
            s2 = *pTable++;
            pTab2 = pTable; /* column indices */
            pTable += s2;
            do_pivot(pB,pTab1,pTab2,j,s1,s2,mod);
            break;
        case RREF:
            i = *pTable++;
            j = *pTable++;
            k = *pTable++;
            l = *pTable++;
            nmod_mat_window_init(*pC, *pB, i, j, i + k, j + l);
            nmod_mat_init(CC, k, l, VOX_Q);
            nmod_mat_set(CC, *pC);
            nmod_mat_rref(CC);
            nmod_mat_set(*pC, CC);
            nmod_mat_clear(CC);
            for (j = 0; j < k; j++)
            {
                singular_system = singular_system || (nmod_mat_entry(*pC, j, j) != 1);
            }
            nmod_mat_window_clear(*pC);
            break;
        case INIT_D:
            i = *pTable++;
            j = *pTable++;
            nmod_mat_init(*pD, i, j, VOX_Q);
            break;
        case COPY_B_D:
            i = *pTable++;
            s1 = *pTable++;
            pTab1 = pTable;
            pTable += s1;
            j = *pTable++;
            s2 = *pTable++;
            pTab2 = pTable;
            pTable += s2;
            assert(s1 == s2);
            for (k = 0; k < s1; k++) {
                nmod_mat_entry(*pD, i, pTab1[k]) = nmod_mat_entry(*pB, j, pTab2[k]);
            }
            break;
        case COPY_A_D:
            i = *pTable++;
            s1 = *pTable++;
            pTab1 = pTable;
            pTable += s1;
            j = *pTable++;
            s2 = *pTable++;
            pTab2 = pTable;
            pTable += s2;
            assert(s1 == s2);
            for (k = 0; k < s1; k++) {
                nmod_mat_entry(*pD, i, pTab1[k]) = nmod_mat_entry(*pA, j, pTab2[k]);
            }
            break;
        case END_LOOP:
            nmod_mat_clear(*pA);
            nmod_mat_clear(*pB);
            {
              nmod_mat_t *pTemp = pA; pA = pD; pD = pB; pB = pC, pC = pTemp;
            }
            break;
        case END:
        default:
            break;
        }
    } while (Tag != END);


    if (singular_system)
    {
        nmod_mat_clear(*pA);
        goto _end;
    }
    if (full == 0)
      return 1;

    assert((*pA)->r == NB_MONOM_HIGH);
    assert((*pA)->c == 2*NB_MONOM_HIGH + NB_MONOM_LOW);

    /*
     * Deduce the Lex basis using FGLM
     */
    nmod_mat_init(E, NB_MONOM_HIGH, TWO_POWER_N, VOX_Q);
    for (i = 0; i < NB_MONOM_HIGH; i++)
    {
        for (j = 0; j < TWO_POWER_N; j++)
        {
            nmod_mat_entry(E, i, j) =
                nmod_neg(nmod_mat_entry(*pA, i, I2[j]), mod);
        }
    }
    sol_ok = 0;
    nmod_mat_clear(*pA);
    nmod_poly_t poly;
    nmod_mat_t F, G, H, I;

    nmod_mat_init(F, TWO_POWER_N, TWO_POWER_N, VOX_Q);
    nmod_mat_entry(F, 0, TWO_POWER_N - 1) = 1;

    nmod_mat_init(H, 1, NB_MONOM_HIGH, VOX_Q);
    for (j = 0; j < TWO_POWER_N - 1; j++)
    {
        for (k = 0; k < NB_MONOM_HIGH; k++)
        {
            nmod_mat_entry(H, 0, k) = nmod_mat_entry(F, j, I3[k]);
        }
        nmod_mat_nmod_vec_mul(F->rows[j + 1], H->rows[0], TWO_POWER_N, E);
        for (k = 0; k < NB_MONOM_LOW; k++)
        {
            nmod_mat_entry(F, j + 1, I5[k]) =
                nmod_add(nmod_mat_entry(F, j + 1, I5[k])
                    , nmod_mat_entry(F, j, I4[k]), mod);
        }
    }

    nmod_mat_init(G, N, TWO_POWER_N, VOX_Q);

    for (k = 0; k < NB_MONOM_HIGH; k++)
    {
        nmod_mat_entry(H, 0, k) = nmod_mat_entry(F, TWO_POWER_N - 1, I3[k]);
    }
    nmod_mat_nmod_vec_mul(G->rows[N - 1], H->rows[0], TWO_POWER_N, E);
    for (k = 0; k < NB_MONOM_LOW; k++)
    {
        nmod_mat_entry(G, N - 1, I5[k]) =
            nmod_add(nmod_mat_entry(G, N - 1, I5[k])
                , nmod_mat_entry(F, TWO_POWER_N - 1, I4[k]), mod);
    }

    nmod_mat_clear(H);
    nmod_mat_clear(E);

    for (i = 0; i < N - 1; i++)
    {
        nmod_mat_entry(G, i, TWO_POWER_N - 1 - N + i) = 1;
    }
    nmod_mat_transpose(F, F);
    nmod_mat_init(H, TWO_POWER_N, N, VOX_Q);
    nmod_mat_transpose(H, G);

    nmod_mat_init(I, TWO_POWER_N, N, VOX_Q);

    slong rank = nmod_mat_solve(I, F, H);

    nmod_mat_clear(F);
    nmod_mat_clear(H);


    if (rank != 0) {
        nmod_mat_transpose(G, I);
        nmod_mat_clear(I);
    }
    else {
        nmod_mat_clear(I);
        nmod_mat_clear(G);
        goto _end;
    }
    if (full == 1)
      return 1;

    /*
     * Univariate resolution
     */
    nmod_poly_init2(poly, VOX_Q, TWO_POWER_N + 1);
    for (i = 0; i < TWO_POWER_N; i++)
    {
        nmod_poly_set_coeff_ui(poly, i, nmod_neg(nmod_mat_entry(G, N - 1, i), mod));
    }
    nmod_poly_set_coeff_ui(poly, TWO_POWER_N, 1);

    if (full == 1)
      return 1;

    mp_limb_t x;
    int res = find_a_root(&x, poly, hint);
    nmod_poly_clear(poly);

    if (res)
    {
        /* Inject the solution into the lex basis to recompute other variables */
        nmod_mat_init(I, N, 1, VOX_Q);
        nmod_mat_entry(I, N - 1, 0) = x;
        nmod_poly_init2(poly, VOX_Q, TWO_POWER_N);
        for (i = 0; i < N - 1; i++)
        {
            for (j = 0; j < TWO_POWER_N; j++)
                nmod_poly_set_coeff_ui(poly, j, nmod_mat_entry(G, i, j));
            nmod_poly_evaluate_nmod_vec(I->rows[i], poly, I->rows[N - 1], 1);
        }
        nmod_poly_clear(poly);
        correct = verify_mq(System, I);
        if (correct == 1)
        {
            sol_ok = 1;
            for (i = 0; i < N; i++)
                nmod_mat_entry(sol, 0, i) = nmod_mat_entry(I, i, 0);
        }
        else
        {
            /* The computed solution was wrong */
            flint_printf("\n Solve_MQ sol_ok KO!! \n");
            sol_ok = 0;
        }
        nmod_mat_clear(I);
    }
    else
    {
        /* No solution */
        sol_ok = 0;
    }
    nmod_mat_clear(G);

_end:
    return sol_ok;
}

