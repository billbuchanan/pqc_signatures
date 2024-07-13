/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include <stdint.h>
#include <stdio.h>
#include "params.h"
#include "polymatrix.h"
#include "polyvec.h"
#include "poly.h"
#include "reduce.h"

void polyvec_matrix_pointwise_product(polyveck c[L], const polyvecl a[K], const polyvecl b[L])
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    polyvec_matrix_pointwise_montgomery(&c[i], a, &b[i]);
}

void polyvec_matrix_pointwise_add(polyvecl c[K], const polyvecl a[K], const polyvecl b[K])
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyvecl_add(&c[i], &a[i], &b[i]);
}

void polyvec_matrix_pointwise_product_l_l(polyvecl c[L], const polyvecl a[L], const polyvecl b[L])
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    polyvec_matrix_pointwise_montgomery_l_l(&c[i], a, &b[i]);
}

void polyvec_matrix_reformat(polyvecl b[K], const polyveck a[L])
{
  unsigned int i, j, k;
  for (i = 0; i < L; ++i)
    for (j = 0; j < K; ++j)
      for (k = 0; k < N; ++k)
        b[j].vec[i].coeffs[k] = a[i].vec[j].coeffs[k];
}

void polyvec_matrix_reformat_l_l(polyvecl b[L], const polyvecl a[L])
{
  unsigned int i, j, k;
  for (i = 0; i < L; ++i)
    for (j = 0; j < L; ++j)
      for (k = 0; k < N; ++k)
        b[j].vec[i].coeffs[k] = a[i].vec[j].coeffs[k];
}

void polymatrix_l_expand(polyvecl v[L], const uint8_t seed[CRHBYTES])
{
  unsigned int i;
  uint16_t nonce;

  for (i = 0; i < L; ++i)
  {
    nonce = (i << 8);
    polyvecl_uniform_eta_g(&v[i], seed, &nonce);
  }
}

void polymatrix_k_l_expand(polyvecl v[K], const uint8_t seed[CRHBYTES])
{
  unsigned int i;
  uint16_t nonce;

  for (i = 0; i < K; ++i)
  {
    nonce = (i << 8);
    polyvecl_uniform_eta_d(&v[i], seed, &nonce);
  }
}

void polyvec_matrix_expand(polyvecl mat[K], const uint8_t rho[SEEDBYTES])
{
  unsigned int i, j;

  for (i = 0; i < K; ++i)
    for (j = 0; j < L; ++j)
      poly_uniform(&mat[i].vec[j], rho, (i << 8) + j);
}

void polyvec_matrix_pointwise_montgomery(polyveck *t, const polyvecl mat[K], const polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyvecl_pointwise_acc_montgomery(&t->vec[i], &mat[i], v);
}

void polyvec_matrix_pointwise_montgomery_l_l(polyvecl *t, const polyvecl mat[L], const polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    polyvecl_pointwise_acc_montgomery(&t->vec[i], &mat[i], v);
}

void polymatrix_ntt_k_l(polyvecl a[K])
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyvecl_ntt(&a[i]);
}

void polymatrix_ntt_l_l(polyvecl a[L])
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    polyvecl_ntt(&a[i]);
}

void polymatrix_invntt_tomont_k_l(polyvecl a[K])
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyvecl_invntt_tomont(&a[i]);
}

void polymatrix_invntt_tomont_l_l(polyvecl a[L])
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    polyvecl_invntt_tomont(&a[i]);
}

void getCofactor(polyvecl b[L], const polyvecl a[L], const int p, const int q, const int n)
{
  unsigned int i = 0, j = 0, k, row, col, tm;
  for (row = 0; row < n; row++)
  {
    for (col = 0; col < n; col++)
    {
      if (row != p && col != q)
      {
        tm = j++;
        for (k = 0; k < N; ++k)
        {
          b[i].vec[tm].coeffs[k] = a[row].vec[col].coeffs[k];
        }
        if (j == n - 1)
        {
          j = 0;
          i++;
        }
      }
    }
  }
}

void determinant(poly *res, const polyvecl a[L], const int size)
{
  unsigned int i, f;

  if (size == 1)
  {
    for (i = 0; i < N; ++i)
    {
      res->coeffs[i] = a[0].vec[0].coeffs[i];
    }
  }
  else
  {
    polyvecl b[L];
    poly sign, det, tmp, sprime;

    for (i = 0; i < N; ++i)
    {
      sign.coeffs[i] = (i == 0 ? 1 : 0);
      sprime.coeffs[i] = (i == 0 ? -1 : 0);
      res->coeffs[i] = 0;
    }

    poly_ntt(&sign);
    poly_ntt(&sprime);
    poly_ntt(res);

    for (f = 0; f < size; f++)
    {
      getCofactor(b, a, 0, f, size);
      poly_pointwise_montgomery(&tmp, &sign, &a[0].vec[f]);
      poly_pointwise_montgomery(&sign, &sign, &sprime);
      determinant(&det, b, size - 1);

      poly_pointwise_montgomery(&det, &tmp, &det);
      poly_add(res, res, &det);
    }
  }
}

S_DOUBLE_Q_SIZE gcdExtended(S_DOUBLE_Q_SIZE a, S_DOUBLE_Q_SIZE b, S_DOUBLE_Q_SIZE *x, S_DOUBLE_Q_SIZE *y)
{
  if (a == 0)
  {
    *x = 0, *y = 1;
    return b;
  }
  S_DOUBLE_Q_SIZE x1, y1;
  S_DOUBLE_Q_SIZE tmp = (b / a) * a;
  S_DOUBLE_Q_SIZE gcd = gcdExtended(b - tmp, a, &x1, &y1);

  *x = y1 - (b / a) * x1;
  *y = x1;

  return gcd;
}

S_DOUBLE_Q_SIZE modInverse(S_DOUBLE_Q_SIZE a)
{
  S_DOUBLE_Q_SIZE x, y;
  DOUBLE_Q_SIZE tmp = addq(a);
  S_DOUBLE_Q_SIZE g = gcdExtended(tmp, Q, &x, &y);
  if (g != 1)
    return -1;
  else
  {
    DOUBLE_Q_SIZE d;
    d = (DOUBLE_Q_SIZE)x;
    d += ((-Q) & ~-((d - (Q >> 1)) >> (DOUBLE_Q_BIT_SIZE - 1))) | (Q & -((d + (Q >> 1)) >> (DOUBLE_Q_BIT_SIZE - 1)));
    return d;
  }
}

void adjoint(polyvecl adj[L], const polyvecl a[L])
{
  unsigned int i, j, k;
  if (L == 1)
  {
    for (i = 0; i < N; ++i)
    {
      adj[0].vec[0].coeffs[i] = (i == 0 ? 1 : 0);
      poly_ntt(&adj[0].vec[0]);
    }
  }
  else
  {
    polyvecl temp[L];
    poly one, minus_one, det;

    for (i = 0; i < N; ++i)
    {
      one.coeffs[i] = (i == 0 ? 1 : 0);
      minus_one.coeffs[i] = (i == 0 ? -1 : 0);
    }
    poly_ntt(&one);
    poly_ntt(&minus_one);

    for (int i = 0; i < L; i++)
    {
      for (int j = 0; j < L; j++)
      {
        getCofactor(temp, a, i, j, L);
        determinant(&det, temp, L - 1);
        if (((i + j) & 1) == 0)
        {
          poly_pointwise_montgomery(&det, &det, &one);
        }
        else
        {
          poly_pointwise_montgomery(&det, &det, &minus_one);
        }

        for (k = 0; k < N; ++k)
        {
          adj[j].vec[i].coeffs[k] = det.coeffs[k];
        }
      }
    }
  }
}

int polymatrix_l_is_invertible(poly *d, const polyvecl v[L])
{
  unsigned int k;
  // d is in ntt format and thus is invertible if all coefficients are nonzero
  determinant(d, v, L);
  for (k = 0; k < N; ++k)
    if (d->coeffs[k] == 0)
      return -1;

  return 0;
}

int polymatrix_l_inverse(polyvecl v[L], const polyvecl u[L])
{
  unsigned int i, j, k;
  poly d;
  polyvecl adj[L];

  if (!polymatrix_l_is_invertible(&d, u))
  {

    adjoint(adj, u);

    for (k = 0; k < N; ++k)
      d.coeffs[k] = (S_Q_SIZE)modInverse(d.coeffs[k]);

    for (i = 0; i < L; i++)
      for (j = 0; j < L; j++)
        poly_pointwise_montgomery(&v[j].vec[i], &adj[i].vec[j], &d);

    return 0;
  }
  return -1;
}

int polymatrix_l_expand_invertible(polyvecl v[L], polyvecl f[L], const uint8_t seed[CRHBYTES])
{
  unsigned int i, j, k = 0;
  poly d;
  polyvecl adj[L];

  polymatrix_l_expand(f, seed);

  if (polymatrix_l_is_invertible(&d, f))
    return -1;

  if (L == 1)
  {
    for (i = 0; i < N; i++)
    {
      v[0].vec[0].coeffs[i] = (S_Q_SIZE)modInverse(d.coeffs[i]);
    }
    return 0;
  }
  adjoint(adj, f);

  for (k = 0; k < N; ++k)
    d.coeffs[k] = (S_Q_SIZE)modInverse(d.coeffs[k]);

  for (i = 0; i < L; i++)
    for (j = 0; j < L; j++)
      poly_pointwise_montgomery(&v[j].vec[i], &adj[i].vec[j], &d);

  return 0;
}
