/* Copyright (C) 2008-2011 Xavier Pujol
   (C) 2015 Michael Walter.
   (C) 2016 Marc Stevens. (generic improvements, auxiliary solutions, subsolutions)
   (C) 2016 Guillaume Bonnoron. (CVP improvements)

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

#include "enumerate_base.h"

FPLLL_BEGIN_NAMESPACE

#ifdef FPLLL_WITH_RECURSIVE_ENUM
template <int kk, int kk_start, bool dualenum, bool findsubsols, bool enable_reset>
inline void EnumerationBase::enumerate_recursive(
    EnumerationBase::opts<kk, kk_start, dualenum, findsubsols, enable_reset>)
{
  enumf alphak  = x[kk] - center[kk];
  enumf newdist = partdist[kk] + alphak * alphak * rdiag[kk];

  if (!(newdist <= partdistbounds[kk]))
    return;
  ++nodes[kk];

  alpha[kk] = alphak;
  if (findsubsols && newdist < subsoldists[kk] && newdist != 0.0)
  {
    subsoldists[kk] = newdist;
    process_subsolution(kk, newdist);
  }

  if (kk == 0)
  {
    if (newdist > 0.0 || !is_svp)
      process_solution(newdist);
  }
  else if (enable_reset &&
           kk < reset_depth)  // in CVP, below the max GS vector, we reset the partial distance
  {
    reset(newdist, kk);
    return;
  }
  else
  {
    partdist[kk - 1] = newdist;
    if (dualenum)
    {
      for (int j = center_partsum_begin[kk]; j > kk - 1; --j)
        center_partsums[kk - 1][j] = center_partsums[kk - 1][j + 1] - alpha[j] * mut[kk - 1][j];
    }
    else
    {
      for (int j = center_partsum_begin[kk]; j > kk - 1; --j)
        center_partsums[kk - 1][j] = center_partsums[kk - 1][j + 1] - x[j] * mut[kk - 1][j];
    }
    if (center_partsum_begin[kk] > center_partsum_begin[kk - 1])
      center_partsum_begin[kk - 1] = center_partsum_begin[kk];
    center_partsum_begin[kk] = kk;
    center[kk - 1]           = center_partsums[kk - 1][kk];
    roundto(x[kk - 1], center[kk - 1]);
    dx[kk - 1] = ddx[kk - 1] = (((int)(center[kk - 1] >= x[kk - 1]) & 1) << 1) - 1;
  }

  while (true)
  {
    FPLLL_TRACE("Level k=" << kk << " dist_k=" << partdist[kk] << " x_k=" << x[kk]
                           << " newdist=" << newdist << " partdistbounds_k=" << partdistbounds[kk]);
    enumerate_recursive(opts<kk - 1, kk_start, dualenum, findsubsols, enable_reset>());

    if (!is_svp || partdist[kk] != 0.0)
    {
      x[kk] += dx[kk];
      ddx[kk] = -ddx[kk];
      dx[kk]  = ddx[kk] - dx[kk];
    }
    else
    {
      ++x[kk];
    }

    enumf alphak2  = x[kk] - center[kk];
    enumf newdist2 = partdist[kk] + alphak2 * alphak2 * rdiag[kk];
    if (!(newdist2 <= partdistbounds[kk]))
      return;
    ++nodes[kk];
    alpha[kk] = alphak2;
    if (kk == 0)
    {
      if (newdist2 > 0.0 || !is_svp)
        process_solution(newdist2);
    }
    else
    {
      partdist[kk - 1] = newdist2;
      if (dualenum)
        center_partsums[kk - 1][kk - 1 + 1] =
            center_partsums[kk - 1][kk - 1 + 1 + 1] - alpha[kk - 1 + 1] * mut[kk - 1][kk - 1 + 1];
      else
        center_partsums[kk - 1][kk - 1 + 1] =
            center_partsums[kk - 1][kk - 1 + 1 + 1] - x[kk - 1 + 1] * mut[kk - 1][kk - 1 + 1];
      if (kk > center_partsum_begin[kk - 1])
        center_partsum_begin[kk - 1] = kk;
      center[kk - 1] = center_partsums[kk - 1][kk - 1 + 1];
      roundto(x[kk - 1], center[kk - 1]);
      dx[kk - 1] = ddx[kk - 1] = (((int)(center[kk - 1] >= x[kk - 1]) & 1) << 1) - 1;
    }
  }
}

#define ENUM_TABLE_FILL8(a)                                                                        \
  &EnumerationBase::enumerate_recursive_wrapper<a + 0, dualenum, findsubsols, enable_reset>,       \
      &EnumerationBase::enumerate_recursive_wrapper<a + 1, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 2, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 3, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 4, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 5, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 6, dualenum, findsubsols, enable_reset>,   \
      &EnumerationBase::enumerate_recursive_wrapper<a + 7, dualenum, findsubsols, enable_reset>
#define ENUM_TABLE_FILL64(a)                                                                       \
  ENUM_TABLE_FILL8(a)                                                                              \
  , ENUM_TABLE_FILL8(a + 8), ENUM_TABLE_FILL8(a + 16), ENUM_TABLE_FILL8(a + 24),                   \
      ENUM_TABLE_FILL8(a + 32), ENUM_TABLE_FILL8(a + 40), ENUM_TABLE_FILL8(a + 48),                \
      ENUM_TABLE_FILL8(a + 56)
#define ENUM_TABLE_FILL256(a)                                                                      \
  ENUM_TABLE_FILL64(a)                                                                             \
  , ENUM_TABLE_FILL64(a + 64), ENUM_TABLE_FILL64(a + 128), ENUM_TABLE_FILL64(a + 192)

template <bool dualenum, bool findsubsols, bool enable_reset>
inline void EnumerationBase::enumerate_recursive_dispatch(int kk)
{
  typedef void (EnumerationBase::*enum_recur_type)();
  static const enum_recur_type lookup[] = {
      ENUM_TABLE_FILL256(0),    ENUM_TABLE_FILL256(256),  ENUM_TABLE_FILL256(512),
      ENUM_TABLE_FILL256(768),  ENUM_TABLE_FILL256(1024), ENUM_TABLE_FILL256(1280),
      ENUM_TABLE_FILL256(1536), ENUM_TABLE_FILL256(1792),
  };
  (this->*lookup[kk])();
}

#endif

template <bool dualenum, bool findsubsols, bool enable_reset> void EnumerationBase::enumerate_loop()
{
  if (k >= k_end)
    return;

  center_partsum_begin[0] = 0;
  for (int i = 0; i < k_end; ++i)
  {
    center_partsum_begin[i + 1] = k_end - 1;
    center_partsums[i][k_end]   = center_partsum[i];
  }

  partdist[k_end] = 0.0;  // needed to make next_pos_up() work properly

  /* The next few lines provide compatibility between the various different enumerators in fplll.
   * Because the recursive enumerator needs to start at one level down from the last position, we
   * decrement it. However, this has the tendency of screwing up the node count by a factor of k
   * (which, although asymptotically shouldn't matter, practically it will). In particular, it has
   * the property of meaning we count an extra node per level (in k+1....k_end-1) in the initial
   * descent. You can think of this as fplll just Babai lifting from k_end->k. However, clearly this
   * may screw up the total - so we adjust this. For more info, see
   * https://github.com/fplll/fplll/pull/442.
   *
   * Note: this adjustment does no checks on the nodes array. This will cause wrap arounds if the
   * values are 0. But, adding to these later will reset them, so this should  not matter in
   * practice.
   */

  for (int i = k + 1; i < k_end; i++)
  {
    nodes[i]--;
  }

  k = k_end - 1;

#ifdef FPLLL_WITH_RECURSIVE_ENUM
  enumerate_recursive_dispatch<dualenum, findsubsols, enable_reset>(k);
  return;
#endif

  finished = false;
  while (!finished)
  {
    enumf alphak  = x[k] - center[k];
    enumf newdist = partdist[k] + alphak * alphak * rdiag[k];
    FPLLL_TRACE("Level k=" << k << " dist_k=" << partdist[k] << " x_k=" << x[k]
                           << " newdist=" << newdist << " partdistbounds_k=" << partdistbounds[k]);
    if (newdist <= partdistbounds[k])
    {
      ++nodes[k];
      alpha[k] = alphak;
      if (findsubsols && newdist < subsoldists[k] && newdist != 0.0)
      {
        subsoldists[k] = newdist;
        process_subsolution(k, newdist);
      }
      --k;
      if (k < 0)
      {
        if (newdist > 0.0 || !is_svp)
          process_solution(newdist);
        finished = !next_pos_up();
        continue;
      }
      if (enable_reset &&
          k < reset_depth)  // in CVP, below the max GS vector, we reset the partial distance
      {
        reset(newdist, k);
        finished = !next_pos_up();
        continue;
      }
      if (dualenum)
      {
        for (int j = center_partsum_begin[k + 1]; j > k; --j)
          center_partsums[k][j] = center_partsums[k][j + 1] - alpha[j] * mut[k][j];
      }
      else
      {
        for (int j = center_partsum_begin[k + 1]; j > k; --j)
          center_partsums[k][j] = center_partsums[k][j + 1] - x[j] * mut[k][j];
      }
      center_partsum_begin[k]     = max(center_partsum_begin[k], center_partsum_begin[k + 1]);
      center_partsum_begin[k + 1] = k + 1;

      enumf newcenter = center_partsums[k][k + 1];
      center[k]       = newcenter;
      partdist[k]     = newdist;
      roundto(x[k], newcenter);
      dx[k] = ddx[k] = (((int)(newcenter >= x[k]) & 1) << 1) - 1;
    }
    else
    {
      finished = !next_pos_up();
    }
  }
}

template void EnumerationBase::enumerate_loop<false, false, true>();
template void EnumerationBase::enumerate_loop<false, true, true>();
template void EnumerationBase::enumerate_loop<false, false, false>();
template void EnumerationBase::enumerate_loop<false, true, false>();
template void EnumerationBase::enumerate_loop<true, false, false>();
template void EnumerationBase::enumerate_loop<true, true, false>();

FPLLL_END_NAMESPACE
