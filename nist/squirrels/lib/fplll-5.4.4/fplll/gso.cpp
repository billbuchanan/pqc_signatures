/* Copyright (C) 2005-2008 Damien Stehle.
   Copyright (C) 2007 David Cade.
   Copyright (C) 2011 Xavier Pujol.

   This file is part of fplll. fplll is free software: you
   can redistribute it and/or modify it under the terms of the GNU Lesser
   General Public License as published by the Free Software Foundation,
   either version 2.1 of the License, or (at your option) any later version.

   fplll is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with fplll. If not, see <http://www.gnu.org/licenses/>. */

/* Template source file */

#include "gso.h"

FPLLL_BEGIN_NAMESPACE

template <class ZT, class FT> void MatGSO<ZT, FT>::update_bf(int i)
{
  int n = max(n_known_cols, init_row_size[i]);
  if (enable_row_expo)
  {
    long max_expo = LONG_MIN;
    for (int j = 0; j < n; j++)
    {
      b(i, j).get_f_exp(bf(i, j), tmp_col_expo[j]);
      max_expo = max(max_expo, tmp_col_expo[j]);
    }
    for (int j = 0; j < n; j++)
    {
      bf(i, j).mul_2si(bf(i, j), tmp_col_expo[j] - max_expo);
    }
    row_expo[i] = max_expo;
  }
  else
  {
    for (int j = 0; j < n; j++)
    {
      bf(i, j).set_z(b(i, j));
    }
  }
}

template <class ZT, class FT> void MatGSO<ZT, FT>::invalidate_gram_row(int i)
{
  for (int j = 0; j <= i; j++)
    gf(i, j).set_nan();
}

template <class ZT, class FT> void MatGSO<ZT, FT>::discover_row()
{
  FPLLL_DEBUG_CHECK(n_known_rows < d);
  /* Early reduction (cols_locked=true) is not allowed when enable_int_gram=true,
     since n_known_cols might be too small to compute all the g(i,j). */
  FPLLL_DEBUG_CHECK(!(cols_locked && enable_int_gram));
  int i = n_known_rows;
  n_known_rows++;
  if (!cols_locked)
  {
    n_source_rows = n_known_rows;
    n_known_cols  = max(n_known_cols, init_row_size[i]);
  }

  if (enable_int_gram)
  {
    for (int j = 0; j <= i; j++)
    {
      b[i].dot_product(g(i, j), b[j], n_known_cols);
    }
  }
  else
  {
    invalidate_gram_row(i);
  }
  gso_valid_cols[i] = 0;
}

template <class ZT, class FT> void MatGSO<ZT, FT>::row_add(int i, int j)
{
  b[i].add(b[j], n_known_cols);
  if (enable_transform)
  {
    u[i].add(u[j]);
    if (enable_inverse_transform)
      u_inv_t[j].sub(u_inv_t[i]);
  }

  if (enable_int_gram)
  {
    // g(i, i) += 2 * g(i, j) + g(j, j)
    ztmp1.mul_2si(sym_g(i, j), 1);
    ztmp1.add(ztmp1, g(j, j));
    g(i, i).add(g(i, i), ztmp1);

    for (int k = 0; k < n_known_rows; k++)
      if (k != i)
        sym_g(i, k).add(sym_g(i, k), sym_g(j, k));
  }
}

template <class ZT, class FT> void MatGSO<ZT, FT>::row_sub(int i, int j)
{
  b[i].sub(b[j], n_known_cols);
  if (enable_transform)
  {
    u[i].sub(u[j]);
    if (enable_inverse_transform)
      u_inv_t[j].add(u_inv_t[i]);
  }

  if (enable_int_gram)
  {
    // g(i, i) += g(j, j) - 2 * g(i, j)
    ztmp1.mul_2si(sym_g(i, j), 1);
    ztmp1.sub(g(j, j), ztmp1);
    g(i, i).add(g(i, i), ztmp1);

    for (int k = 0; k < n_known_rows; k++)
      if (k != i)
        sym_g(i, k).sub(sym_g(i, k), sym_g(j, k));
  }
}

template <class ZT, class FT> void MatGSO<ZT, FT>::row_addmul_si(int i, int j, long x)
{
  b[i].addmul_si(b[j], x, n_known_cols);
  if (enable_transform)
  {
    u[i].addmul_si(u[j], x);
    if (enable_inverse_transform)
      u_inv_t[j].addmul_si(u_inv_t[i], -x);
  }

  if (enable_int_gram)
  {
    /* g(i, i) += 2 * x * g(i, j) +  x^2 * g(j, j)
      (must be done before updating g(i, j)) */
    ztmp1.mul_si(sym_g(i, j), x);
    ztmp1.mul_2si(ztmp1, 1);
    g(i, i).add(g(i, i), ztmp1);
    ztmp1.mul_si(g(j, j), x);
    ztmp1.mul_si(ztmp1, x);
    g(i, i).add(g(i, i), ztmp1);

    // g(i, k) += g(j, k) * x  for k != i
    for (int k = 0; k < n_known_rows; k++)
    {
      if (k == i)
        continue;
      ztmp1.mul_si(sym_g(j, k), x);
      sym_g(i, k).add(sym_g(i, k), ztmp1);
    }
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::row_addmul_si_2exp(int i, int j, long x, long expo)
{
  b[i].addmul_si_2exp(b[j], x, expo, n_known_cols, ztmp1);
  if (enable_transform)
  {
    u[i].addmul_si_2exp(u[j], x, expo, ztmp1);
    if (enable_inverse_transform)
      u_inv_t[j].addmul_si_2exp(u_inv_t[i], -x, expo, ztmp1);
  }

  if (enable_int_gram)
  {
    /* g(i, i) += 2 * (2^e * x) * g(i, j) + 2^(2*e) * x^2 * g(j, j)
      (must be done before updating g(i, j)) */
    ztmp1.mul_si(sym_g(i, j), x);
    ztmp1.mul_2si(ztmp1, expo + 1);
    g(i, i).add(g(i, i), ztmp1);
    ztmp1.mul_si(g(j, j), x);
    ztmp1.mul_si(ztmp1, x);
    ztmp1.mul_2si(ztmp1, 2 * expo);
    g(i, i).add(g(i, i), ztmp1);

    // g(i, k) += g(j, k) * (2^e * x) for k != i
    for (int k = 0; k < n_known_rows; k++)
    {
      if (k == i)
        continue;
      ztmp1.mul_si(sym_g(j, k), x);
      ztmp1.mul_2si(ztmp1, expo);
      sym_g(i, k).add(sym_g(i, k), ztmp1);
    }
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::row_addmul_2exp(int i, int j, const ZT &x, long expo)
{
  b[i].addmul_2exp(b[j], x, expo, n_known_cols, ztmp1);
  if (enable_transform)
  {
    u[i].addmul_2exp(u[j], x, expo, ztmp1);
    if (enable_inverse_transform)
    {
      ZT minus_x;
      minus_x.neg(x);
      u_inv_t[j].addmul_2exp(u_inv_t[i], minus_x, expo, ztmp1);
    }
  }

  if (enable_int_gram)
  {
    /* g(i, i) += 2 * (2^e * x) * g(i, j) + 2^(2*e) * x^2 * g(j, j)
      (must be done before updating g(i, j)) */
    ztmp1.mul(sym_g(i, j), x);
    ztmp1.mul_2si(ztmp1, expo + 1);
    g(i, i).add(g(i, i), ztmp1);
    ztmp1.mul(g(j, j), x);
    ztmp1.mul(ztmp1, x);
    ztmp1.mul_2si(ztmp1, 2 * expo);
    g(i, i).add(g(i, i), ztmp1);

    // g(i, k) += g(j, k) * (2^e * x) for k != i
    for (int k = 0; k < n_known_rows; k++)
    {
      if (k == i)
        continue;
      ztmp1.mul(sym_g(j, k), x);
      ztmp1.mul_2si(ztmp1, expo);
      sym_g(i, k).add(sym_g(i, k), ztmp1);
    }
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::row_addmul_we(int i, int j, const FT &x, long expo_add)
{
  FPLLL_DEBUG_CHECK(j >= 0 && /* i > j &&*/ i < n_known_rows && j < n_source_rows);

  long expo;
  long lx = x.get_si_exp_we(expo, expo_add);

  if (expo == 0)
  {
    if (lx == 1)
      row_add(i, j);
    else if (lx == -1)
      row_sub(i, j);
    else if (lx != 0)
      row_addmul_si(i, j, lx);
  }
  else if (row_op_force_long)
  {
    row_addmul_si_2exp(i, j, lx, expo);
  }
  else
  {
    x.get_z_exp_we(ztmp2, expo, expo_add);
    row_addmul_2exp(i, j, ztmp2, expo);
  }
}
// In row_swap, i < j
template <class ZT, class FT> void MatGSO<ZT, FT>::row_swap(int i, int j)
{
  FPLLL_DEBUG_CHECK(!enable_inverse_transform);
  b.swap_rows(i, j);
  if (enable_transform)
  {
    u.swap_rows(i, j);
  }

  if (enable_int_gram)
  {
    if (j < i)  // Leaving this check here?
    {
      throw std::runtime_error("Error: in row_swap, i > j, causing errors in the grammatrix.");
    }
    for (int k = 0; k < i; k++)
      g(i, k).swap(g(j, k));
    for (int k = i + 1; k < j; k++)
      g(k, i).swap(g(j, k));
    for (int k = j + 1; k < n_known_rows; k++)
      g(k, i).swap(g(k, j));
    g(i, i).swap(g(j, j));
  }
}

template <class ZT, class FT> void MatGSO<ZT, FT>::move_row(int old_r, int new_r)
{
  FPLLL_DEBUG_CHECK(!cols_locked);
  if (new_r < old_r)
  {
    FPLLL_DEBUG_CHECK(old_r < n_known_rows && !cols_locked);
    for (int i = new_r; i < n_known_rows; i++)
    {
      invalidate_gso_row(i, new_r);
    }
    rotate(gso_valid_cols.begin() + new_r, gso_valid_cols.begin() + old_r,
           gso_valid_cols.begin() + old_r + 1);
    mu.rotate_right(new_r, old_r);
    r.rotate_right(new_r, old_r);
    b.rotate_right(new_r, old_r);
    if (enable_transform)
    {
      u.rotate_right(new_r, old_r);
      if (enable_inverse_transform)
        u_inv_t.rotate_right(new_r, old_r);
    }
    if (enable_int_gram)
    {
      g.rotate_gram_right(new_r, old_r, n_known_rows);
    }
    else
    {
      gf.rotate_gram_right(new_r, old_r, n_known_rows);
      bf.rotate_right(new_r, old_r);
    }
    if (enable_row_expo)
      rotate(row_expo.begin() + new_r, row_expo.begin() + old_r, row_expo.begin() + old_r + 1);
  }
  else if (new_r > old_r)
  {
    for (int i = old_r; i < n_known_rows; i++)
    {
      invalidate_gso_row(i, old_r);
    }
    rotate(gso_valid_cols.begin() + old_r, gso_valid_cols.begin() + old_r + 1,
           gso_valid_cols.begin() + new_r + 1);
    mu.rotate_left(old_r, new_r);
    r.rotate_left(old_r, new_r);
    b.rotate_left(old_r, new_r);
    if (enable_transform)
    {
      u.rotate_left(old_r, new_r);
      if (enable_inverse_transform)
        u_inv_t.rotate_left(old_r, new_r);
    }
    if (enable_int_gram)
    {
      if (old_r < n_known_rows - 1)
      {
        g.rotate_gram_left(old_r, min(new_r, n_known_rows - 1), n_known_rows);
      }
    }
    else
    {
      if (old_r < n_known_rows - 1)
        gf.rotate_gram_left(old_r, min(new_r, n_known_rows - 1), n_known_rows);
      bf.rotate_left(old_r, new_r);
    }
    if (enable_row_expo)
      rotate(row_expo.begin() + old_r, row_expo.begin() + old_r + 1, row_expo.begin() + new_r + 1);
    if (new_r >= n_known_rows)
    {
      rotate(init_row_size.begin() + old_r, init_row_size.begin() + old_r + 1,
             init_row_size.begin() + new_r + 1);
      if (old_r < n_known_rows)
      {
        n_known_rows--;
        n_source_rows        = n_known_rows;
        init_row_size[new_r] = max(b[new_r].size_nz(), 1);
      }
    }
  }
}

template <class ZT, class FT> void MatGSO<ZT, FT>::size_increased()
{
  int old_d = mu.get_rows();

  if (d > alloc_dim)
  {
    if (enable_int_gram)
    {
      g.resize(d, d);
    }
    else
    {
      bf.resize(d, b.get_cols());
      gf.resize(d, d);
    }
    mu.resize(d, d);
    r.resize(d, d);
    gso_valid_cols.resize(d);
    init_row_size.resize(d);
    if (enable_row_expo)
    {
      row_expo.resize(d);
    }
    alloc_dim = d;
  }

  for (int i = old_d; i < d; i++)
  {
    init_row_size[i] = max(b[i].size_nz(), 1);
    if (!enable_int_gram)
    {
      bf[i].fill(0);  // update_bf might not copy all the zeros of b[i]
      update_bf(i);
    }
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::to_canonical(vector<FT> &w, const vector<FT> &v, long start)
{
  vector<FT> x = vector<FT>(v);
  long dim     = (((long)x.size()) < d - start) ? x.size() : d - start;
  long expo    = 0;
  FT tmp       = 0.0;
  // 1. triangular system solving
  for (long i = dim - 1; i >= 0; i--)
  {
    for (long j = i + 1; j < dim; j++)
    {
      get_mu(tmp, start + j, start + i);
      x[i] -= tmp * x[j];
    }
  }
  // 2. multiply by B
  w.resize(b.get_cols());
  for (long j = 0; j < b.get_cols(); j++)
  {
    w[j] = 0.0;
    for (long i = 0; i < dim; i++)
    {
      b[start + i][j].get_f_exp(tmp, expo);
      tmp.mul(tmp, x[i]);
      tmp.mul_2si(tmp, expo);
      w[j] += tmp;
    }
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::from_canonical(vector<FT> &v, const vector<FT> &w, long start, long dimension)
{
  long dim  = (dimension == -1) ? d - start : dimension;
  long expo = 0;
  FT tmp    = 0.0;
  vector<FT> x(start + dim);

  // 1.multiply by B
  for (long i = 0; i < start + dim; i++)
  {
    x[i] = 0.0;
    for (long j = 0; j < b.get_cols(); j++)
    {
      b[i][j].get_f_exp(tmp, expo);
      tmp.mul(tmp, w[j]);
      tmp.mul_2si(tmp, expo);
      x[i] += tmp;
    }
  }
  // 2. triangular system solving
  for (long i = 0; i < start + dim; i++)
  {
    for (long j = 0; j < i; j++)
    {
      get_mu(tmp, i, j);
      tmp.mul(tmp, x[j]);
      x[i] -= tmp;
    }
  }
  // 3. scale
  for (long i = 0; i < dim; i++)
  {
    get_r(tmp, start + i, start + i);
    x[i + start] /= tmp;
  }

  // copy out
  v.resize(dim);
  for (long i = 0; i < dim; i++)
  {
    v[i] = x[start + i];
  }
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::babai(vector<ZT> &v, int start, int dimension, bool gso)
{
  vector<FT> w;
  FT tmp = 0.0;
  for (size_t i = 0; i < v.size(); i++)
  {
    tmp.set_z(v[i]);
    w.push_back(tmp);
    v[i] = 0;
  }
  babai(v, w, start, dimension, gso);
}

template <class ZT, class FT>
void MatGSO<ZT, FT>::babai(vector<ZT> &w, const vector<FT> &v, int start, int dimension, bool gso)
{
  dimension = (dimension == -1) ? this->d - start : dimension;
  vector<FT> x;
  if (!gso)
  {
    from_canonical(x, v, start, dimension);
    MatGSOInterface<ZT, FT>::babai(w, x, start, dimension);
  }
  else
  {
    MatGSOInterface<ZT, FT>::babai(w, v, start, dimension);
  }
}

template class MatGSO<Z_NR<long>, FP_NR<double>>;
template class MatGSO<Z_NR<double>, FP_NR<double>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<double>>;

#ifdef FPLLL_WITH_LONG_DOUBLE
template class MatGSO<Z_NR<long>, FP_NR<long double>>;
template class MatGSO<Z_NR<double>, FP_NR<long double>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<long double>>;

#endif

#ifdef FPLLL_WITH_QD
template class MatGSO<Z_NR<long>, FP_NR<dd_real>>;
template class MatGSO<Z_NR<double>, FP_NR<dd_real>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<dd_real>>;

template class MatGSO<Z_NR<long>, FP_NR<qd_real>>;
template class MatGSO<Z_NR<double>, FP_NR<qd_real>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<qd_real>>;
#endif

#ifdef FPLLL_WITH_DPE
template class MatGSO<Z_NR<long>, FP_NR<dpe_t>>;
template class MatGSO<Z_NR<double>, FP_NR<dpe_t>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<dpe_t>>;
#endif

template class MatGSO<Z_NR<long>, FP_NR<mpfr_t>>;
template class MatGSO<Z_NR<double>, FP_NR<mpfr_t>>;
template class MatGSO<Z_NR<mpz_t>, FP_NR<mpfr_t>>;

FPLLL_END_NAMESPACE
