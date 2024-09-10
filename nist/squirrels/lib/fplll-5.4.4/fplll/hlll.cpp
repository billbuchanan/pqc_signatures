/* Copyright (C) 2005-2008 Damien Stehle.
   Copyright (C) 2007 David Cade.
   Copyright (C) 2011 Xavier Pujol.
   Copyright (C) 2017-1018 Laurent Grémy.

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

#include "hlll.h"
#include "util.h"

FPLLL_BEGIN_NAMESPACE

template <class ZT, class FT> bool HLLLReduction<ZT, FT>::hlll()
{
  /* TODO: we do not use a completely correct value for delta. We must use a value
   * delta_ in (delta + 2^(-p + p0), 1 - 2^(-p + p0)). This implies that, with the
   * fast method, we cannot guarantee the HLLL-reduction w.r.t delta.
   *
   * A careful analysis about how to set delta may help to obtain this guarantee.
   * Experimentally, the delta used during the computation must be equal to
   * (delta + (0.01 or 0.02) to have an basis which is HLLL-reduced w.r.t delta.
   */
  int start_time = (verbose) ? cputime() : 0;
  // True if the corresponding vector is weak size-reduced
  bool status_sr = true;

  if (verbose)
  {
    // Print the parameters of the computation
    print_params();
    // Discover b[0]
    cerr << "Discovering vector 1/" << m.get_d() << " cputime=" << cputime() - start_time << endl;
  }

  // Set R[0] and bf[0] to b[0], precompute ||b[0]||^2
  m.refresh_R_bf(0);
  // Compute R[0]
  m.update_R_last(0);  // In this case, update_R(0) is exactly equal to update_R_last(0)
  // Precompute dR[0]: R[0]^2 * delta = dR[0] * 2^(2*row_expo[0])
  compute_dR(0);
  // Precompute eR[0]: R[0] * eta = eR[0] * 2^row_expo[0]
  compute_eR(0);

  int k = 1;
  // Remember which was the largest b[k_max] that is tried to be size-reduced
  int k_max = 1;

  int prev_k = -1;
  vector<FT> prev_R;
  vector<long> prev_expo;
  prev_R.resize(m.get_d());
  prev_expo.resize(m.get_d());
  // prev_R[0] is never used: m.get_R(prev_R[0], 0, 0, prev_expo[0]);

  if (verbose)
  {
    // Discover b[1]
    cerr << "Discovering vector 2/" << m.get_d() << " cputime=" << cputime() - start_time << endl;
  }

  // Set R[1] and bf[1] to b[1], precompute ||b[1]||^2
  m.refresh_R_bf(1);

  while (true)
  {
    // Size reduce b[k] thanks to b[0] to b[k - 1]
    size_reduction(k, k, 0);
    status_sr = verify_size_reduction(k);  // b[kappa] should be size_reduced. Verify it.
    if (!status_sr)
      return set_status(RED_HLLL_SR_FAILURE);

    if (lovasz_test(k))
    {
      // Fully compute R[k], since all the coefficient except one of R[k] were computed during
      // size_reduction.
      m.update_R_last(k);
      // Compute delta_ * R(k, k)^2 = dR[k] * 2^(2*row_expo[k])
      compute_dR(k);
      // Compute eta * R(k, k) = eR[k] * 2^row_expo[k]
      compute_eR(k);

      // Heuristic precision check : when R(kappa-1,kappa-1) increases in a 2x2 up and down (see
      // hplll)
      if (prev_k == k + 1)
      {
        // R(k, k) = ftmp0 * 2^expo0
        m.get_R(ftmp0, k, k, expo0);
        ftmp1.mul_2si(prev_R[k], prev_expo[k] - expo0);

        if (ftmp0.cmp(ftmp1) > 0)
        {
          // cerr << "Anomaly: the norm increases for kappa = " << k << endl;
          return set_status(RED_HLLL_NORM_FAILURE);
        }
      }

      prev_k = k;
      // R(k, k) = prev_R[k] * 2^prev_expo[k]
      m.get_R(prev_R[k], k, k, prev_expo[k]);

      // b[k] is size reduced, now, size reduce b[k + 1]
      k++;

      if (k < m.get_d())
      {
        if (k > k_max)
        {
          // First time b[k] is discovered
          if (verbose)
          {
            cerr << "Discovering vector " << k + 1 << "/" << m.get_d()
                 << " cputime=" << cputime() - start_time << endl;
          }
          k_max = k;
          // Set R[k] and bf[k] to b[k], precompute ||b[k]||^2
          m.refresh_R_bf(k);
        }
        else
          // Set R[k] to b[k]. Indeed, it is not necessary to refresh bf[k], since b[k] has not
          // changed. However, it is mandatory to refresh R[k], since b[0] to b[k - 1] may have
          // changed, and then, it is necessary to recompute R[k].
          m.refresh_R(k);
      }
      else
        // if k == m.get_d(), then b[k] does not exist and the computation is ended
        return set_status(RED_SUCCESS);
    }
    else
    {
      // Swap b[k-1] and b[k] and other usefull variables
      m.swap(k - 1, k);
      prev_k = k;

      if (k - 1 == 0)
      {
        // Set R[0] to b[0] (bf[0] and other usefull variables were swaped previously)
        m.refresh_R(0);
        // Compute R[0]
        m.update_R_last(0);  // In this case, update_R(0) is exactly equal to update_R_last(0)
        // Precompute dR[0]: R[0]^2 * delta = dR[0] * 2^(2*row_expo[0])
        compute_dR(0);
        // Precompute eR[0]: R[0] * eta = eR[0] * 2^row_expo[0]
        compute_eR(0);

        // Set R[1] to b[1] (bf[1] and other usefull variables were swaped previously)
        m.refresh_R(1);
        k = 1;
      }
      else
      {
        // Size reduce b[k - 1]
        k--;
        // Since b[k] was not changed, a previous computation of R[k][0..k-1] can be used instead of
        // recomputing R[k][0..k-1], which is the only interesting part to begin the size reduction
        // of b[k]
        m.recover_R(k);
      }
    }
  }
}

template <class ZT, class FT> bool HLLLReduction<ZT, FT>::lovasz_test(int k)
{
#ifndef MODIFIED_LOVASZ_TEST
  // This Lovasz test is the one proposed in [MSV, ISSAC'09]
  //
  // Prior, we used another test, which is dR[k-1].cmp(R(k, k - 1)^2 + R(k, k)^2) <= 0, if R(k, k)
  // is known (which is not the case here at this step of the computation, but can be computed
  // thanks to sqrt(sum_{i=k}^{i<n}R(k, i)^2) (indices must be checked)). In the prior version,
  // since R(k, k)^2 was known, we directly used the test. However, the test was probably not as
  // accurate as what we hope. However, since this formula is used in hplll
  // (https://github.com/gilvillard/hplll/releases), this can maybe be retested. An example of
  // matrices that was not HLLL reduced because of a fail in this test can be generated thanks to
  // latticegen -randseed 122 r 300 30000.
  // Such a test can be activate by compiling with -DMODIFIED_LOVASZ_TEST

  m.get_norm_square_b(ftmp0, k, expo0);  // ||b[k]||^2 = ftmp0 * 2^expo0
  m.norm_square_R_row(ftmp1, k, 0, k - 1,
                      expo1);  // sum_{i = 0}^{i < k - 1}R[k][i]^2 = ftmp1 * 2^expo1

  // If this check is false, we need to reenable
  // ftmp0.mul_2si(ftmp0, expo0 - expo1);
  FPLLL_DEBUG_CHECK(expo0 == expo1);

  ftmp1.sub(ftmp0, ftmp1);  // ||b[k]||^2 - sum_{i = 0}^{i < k - 1}R[k][i]^2 = ftmp1 * 2^expo1

  expo0 = m.get_row_expo(k - 1);

  // Here, delta * R(k - 1, k - 1)^2 = dR[k-1] * 2^(2*expo0). We want to compare
  //   delta * R(k - 1, k - 1)^2 <= ||b[k]||^2 - sum_{i = 0}^{i < k - 1}R[k][i]^2
  //   dR[k-1] * 2^(2*expo0) <= ftmp1 * 2^expo1
  //   dR[k-1] <= ftmp1 * 2^(expo1 - 2*expo0)
  ftmp1.mul_2si(ftmp1, expo1 - 2 * expo0);
#else  // MODIFIED_LOVASZ_TEST
  // Modified Lovasz test, following the comment above.
  // FIXME: probably not maintained, especially if code is not compiled with -DMODIFIED_LOVASZ_TEST
  // for a while.

  m.norm_square_R_row(ftmp1, k, k, m.get_n(),
                      expo1);  // sum_{i = k}^{i < n}R[k][i]^2 = ftmp1 * 2^expo1
// Since we do not necessarily need expo1 (since expo0 and expo1 are correlated), we can
// avoid to return it.
#ifdef DEBUG
  m.get_R(ftmp0, k, k - 1, expo0);  // R(k, k - 1) = ftmp0 * 2^expo0
#else   // DEBUG
  m.get_R(ftmp0, k, k - 1);  // R(k, k - 1) = ftmp0 * 2^(expo1 / 2)
#endif  // DEBUG

  ftmp0.mul(ftmp0, ftmp0);  // R(k, k - 1)^2 = ftmp0 * 2^(2 * expo 0)

  // If this check is false, we need to reenable
  // ftmp0.mul_2si(ftmp0, 2 * expo0 - expo1);  // 2 * expo0 since R(k, k-1)^2 = ftmp0 *
  // (2^expo0)^2
  FPLLL_DEBUG_CHECK(2 * expo0 == expo1);

  ftmp1.add(ftmp0, ftmp1);  // sum_{i = k}^{i < n}R[k][i]^2 + R(k, k-1)^2 = ftmp1 * 2^expo1

  expo0 = m.get_row_expo(k - 1);

  // Here, delta * R(k - 1, k - 1)^2 = dR[k-1] * 2^(2*expo0). We want to compare
  //   delta * R(k - 1, k - 1)^2 <= sum_{i = k}^{i < n}R[k][i]^2 + R(k, k-1)^2
  //   dR[k-1] * 2^(2*expo0) <= ftmp1 * 2^expo1
  //   dR[k-1] <= ftmp1 * 2^(expo1 - 2*expo0)
  ftmp1.mul_2si(ftmp1, expo1 - 2 * expo0);
#endif  // MODIFIED_LOVASZ_TEST

  return (dR[k - 1].cmp(ftmp1) <= 0);
}

// TODO: there is maybe a problem. It seems that size_reduction is often (maybe always) stopped
// because of reduced is false and almost never because the conditions about the norm of b[kappa]
// are reached. This can be an issue.

/*
 * EXPLANATION:
 *
 * The incomplete size-reduction algorithm (Algorithm 3 in HLLL) performs several step to
 * size-reduce b[kappa]. However, we remarked that the condition to break the do...until loop is
 * not sufficient to guarantee that b[kappa] is weak size-reduced. We then modify the test, but now,
 * this test seems never being reached (at least in this implementation) and we leave the
 * size_reduction function almost only when b[kappa] remains unchanged, instead of leaving the loop
 * when the progress is sufficient.
 *
 * My guess is that it can be a problem coming from this implementation, or coming from the test to
 * stop the loop, since it is very strict. I think that it can be seen as a limitation of the code,
 * and a way to improve it is to find clever bound to stop the loop depending on the norms of
 * b[kappa].
 */
template <class ZT, class FT>
void HLLLReduction<ZT, FT>::size_reduction(int kappa, int size_reduction_end,
                                           int size_reduction_start)
{
  FPLLL_DEBUG_CHECK(kappa >= size_reduction_end);
  FPLLL_DEBUG_CHECK(size_reduction_start < size_reduction_end);
  FPLLL_DEBUG_CHECK(0 <= size_reduction_start);

  // If b[kappa] is reduced by at least one b[i], then reduced will be set to true.
  bool reduced = false;

  /*
   * Variables introduced in hplll (https://github.com/gilvillard/hplll)
   * See commit a6b29d1a23ca34000264e22608ef23a64e3cac9d
   * It seems that testing only one time the condition on line 7 of Algorithm 3 (incomplete size
   * reduction) is not sufficient to ensure that b[kappa] is size-reduced. We then test it two
   * times. If two times in a row, the condition is reached, then we consider that b[kappa] is
   * size-reduced.
   */
  bool not_stop      = true;
  bool prev_not_stop = true;

#ifndef HOUSEHOLDER_USE_SIZE_REDUCTION_TEST
  /*
   * Variables introduced in hplll (https://github.com/gilvillard/hplll)
   * See commit a6b29d1a23ca34000264e22608ef23a64e3cac9d
   * The loop is done while ||bk||^2 <= a * t, where t is the squared norm of bk before size
   * reduction and a = 2^(-cd).
   *   The condition is equivalent to -1/2 * log(a) <= 1/2*log(t) - log(||bk||), where 1/2*log(t) is
   *   the log of the squared norm of bk before size reduction. For c=0.1 and d=300,
   *   -1/2 * log_2(a) = 15, then the loop continues when the length of bk decreases from more than
   *   15 bits. For a=0.1, the same thing occurs with less than 2 bits. It experimentally allows to
   *   size reduce the vectors for dimension >= 260.
   * TODO: this hard coded value must be theoretically analyzed.
   */
  FT approx = 0.1;
#else   // HOUSEHOLDER_USE_SIZE_REDUCTION_TEST
  FT approx = sr;
#endif  // HOUSEHOLDER_USE_SIZE_REDUCTION_TEST

  m.update_R(kappa, false);

  /* Most likely, at this step, the next update_R(kappa, false) must modify some coefficients since
   * b will most likely be changed. If b is not modified during the size reduction, there will be
   * only a call to update_R_last(kappa), which automatically must set updated_R to false. We set
   * updated_R to false since it can be equal to true if recover_R was called that allows to avoid
   * an unuseful recomputation of R[kappa] with update_R.
   * TODO: maybe find a best place to use this function.
   */
  m.set_updated_R_false();

  do
  {
    reduced = m.size_reduce(kappa, size_reduction_end, size_reduction_start);

    // If not reduced, b(kappa) has not changed. Computing ||b[kappa]||^2 is not necessary.
    // 1 > 2^(-cd)=sr since cd > 0.
    if (!reduced)
      return;
    else
    {
      // At this point, even if b has changed, the precomputed squared norm of b was for b before
      // the reduction
      m.get_norm_square_b(ftmp0, kappa, expo0);  // ||b[kappa]||^2 = t = ftmp0 * 2^expo
      // Since b has changed, R must be recomputed (latter in the implementation) and then R[kappa]
      // and bf[kappa] are set to b[kappa]. The squared norm of b is updated, then, the next call to
      // get_norm_square_b(..., kappa, ...) will get the squared norm of the current b.
      m.refresh_R_bf(kappa);
      m.get_norm_square_b(ftmp1, kappa, expo1);  // ||b[kappa]||^2 = ftmp1 * 2^expo1

      ftmp0.mul(approx, ftmp0);  // approx * t = ftmp0 * 2^expo0

      // TODO: Why not doing ftmp1.mul_2si(ftmp1, expo1 - expo0); ?
      // We want to compare
      //   ||b[k]||^2 > approx * t
      //   ftmp1 * 2^expo1 > ftmp0 * 2^expo0
      //   ftmp1 > ftmp0 * 2^(expo0-expo1)
      ftmp0.mul_2si(ftmp0, expo0 - expo1);

      // If (||b(kappa)||^2 > approx * t => ftmp1 > ftmp0), stop the loop.
      not_stop = (ftmp1.cmp(ftmp0) <= 0);

      // Update R(kappa, 0..kappa-1).
      m.update_R(kappa, false);

      if (prev_not_stop || not_stop)
        prev_not_stop = not_stop;  // Continue to try to reduce b(kappa).
      else
        return;
    }
  } while (true);
}

/*
 * A first version of this test was introduced in 2015-04-09 in hplll (see
 * commit 93da15d1418347714ef5c07ae8860946825772e5). This test detects not
 * enough precision during the computation is therefore needed in addition to
 * the one about the norms (such a test is implemented and send a
 * RED_HLLL_NORM_FAILURE in this implementation).
 *
 * However, for now on, the following test about the weak size reduction was
 * never used to stop one of our computations, even if hplll, for a same lattice,
 * stops by reaching this test.
 *
 * TODO: is the following test actually used to detect an hypothetical infinite
 * loop or not?
 * HYPOTHESIS: this test was used previously when eta=0.52 and theta = 0.01. Since now
 *   eta=0.51 and theta=0.001, maybe this test will be used.
 */
template <class ZT, class FT> bool HLLLReduction<ZT, FT>::verify_size_reduction(int kappa)
{
#ifdef HOUSEHOLDER_VERIFY_SIZE_REDUCTION_HPLLL
  /*
   * This test tries to mimick to the test of hplll in hsizereduce. It tests if
   *   |R(k, i)| / R(i, i) <= (0.00...01 * ||b[kappa]||) / R(i, i) + 1
   * but we test with this one
   *   |R(k, i)| <= 0.00...01 * ||b[kappa]|| + R(i, i)
   */

  // TODO: can this test be more concise. This test is used at the end of each call to
  // size_reduction, improving it can save time, especially if, for now, it does not help
  // to detect loss of precision during the computation.

  m.get_norm_square_b(ftmp0, kappa, expo0);  // ||b[kappa]||^2 = ftmp0 * 2^expo0

  FPLLL_DEBUG_CHECK(expo0 % 2 == 0);
  ftmp0.sqrt(ftmp0);
  expo0 = expo0 / 2;  // ||b[kappa]|| = ftmp0 * 2^expo0

  ftmp2 = 0.00000000001;
  ftmp0.mul(ftmp0, ftmp2);  // See hplll, to tune for theta depending on the precision.
  // 0.00000000001 * ||b[kappa]|| = ftmp0 * 2^expo0

  for (int i = 0; i < kappa; i++)
  {
// Since we do not necessarily need expo1 (since expo0 and expo1 are correlated), we can
// avoid to return it.
#ifdef DEBUG
    m.get_R(ftmp1, kappa, i, expo1);  // R(kappa, i) = ftmp1 * 2^expo1
#else                                 // DEBUG
    m.get_R(ftmp1, kappa, i);  // R(kappa, i) = ftmp1 * 2^expo0
#endif                                // DEBUG

    FPLLL_DEBUG_CHECK(expo0 == expo1);  // Since R[kappa] and b[kappa] share the same row_expo.
    ftmp1.abs(ftmp1);

    m.get_R(ftmp2, i, i, expo2);  // R(i, i) = ftmp2 * 2^expo2

#if 0
    // We want to test if
    //   |R(kappa, i)| <= (0.00...01 * ||b[kappa]||) + R(i, i)
    //   ftmp1 * 2^expo1 <= ftmp0 * 2^expo1 + ftmp2 * 2^expo2, since expo0 == expo1
    //   ftmp1 <= ftmp0 + ftmp2 * 2^(expo2 - expo1);

    ftmp2.mul_2si(ftmp2, expo2 - expo1);
    ftmp2.add(ftmp0, ftmp2);

#else   // 0
    // We want to test if (theoretically the same test as in hplll)
    //   |R(kappa, i)| / R(i, i) <= (0.00...01 * ||b[kappa]||) / R(i, i) + 1
    //   ftmp1 / ftmp2 * 2^(expo1 - expo2) <= ftmp0 / ftmp2 * 2^(expo0 - expo2) + 1
    //   ftmp1 / ftmp2 <= ftmp0 / ftmp2 + 2^(expo2 - expo1), since expo0 == expo1

    ftmp1.div(ftmp1, ftmp2);
    // Here, |R(kappa, i)| / R(i, i) = ftmp1 * 2^(expo1 - expo2)

    ftmp2.div(ftmp0, ftmp2);
    // Here, (0.00...01 * ||b[kappa]||) / R(i, i) = ftmp2 * 2^(expo1 - expo2);

    FT one = 1.0;
    one.mul_2si(one, expo2 - expo1);
    ftmp2.add(one, ftmp2);
#endif  // 0

    if (ftmp1.cmp(ftmp2) > 0)
    {
#ifdef HOUSEHOLDER_DEBUG
      cerr << "This is probably the first time this test is used." << endl;
#endif  // HOUSEHOLDER_DEBUG

      cerr << "Anomaly: weak size reduction is not complete kappa = " << kappa << " and i = " << i
           << endl;

      return false;
    }
  }
#else  // HOUSEHOLDER_VERIFY_SIZE_REDUCTION_HPLLL
  /*
   * This test is similar to the test of hplll in hsizereduce. It is however not
   * exactly the same, this one crudely verify the condition of the weak-size
   * reduction. The one of hplll verify if
   *   |R(k, i)| / R(i, i) <= (0.00...01 * ||b[kappa]||) / R(i, i) + 1
   */

  // TODO: can this test be more concise. This test is used at the end of each call to
  // size_reduction, improving it can save time, especially if, for now, it does not help
  // to detect loss of precision during the computation.

  // Since R(kappa, kappa) is not know at this time, compute its value
  // For now on, R(kappa, kappa) is assumed to be known, even if the value stored
  // at R(kappa, kappa) is not correct.
  m.norm_R_row(ftmp1, kappa, kappa, m.get_n(), expo1);  // R(kappa, kappa) = ftmp1 * 2^expo1

  ftmp1.mul(ftmp1, theta);  // theta * R(kappa, kappa) = ftmp1 * 2^expo1

  // Verify the conditions on the weak size-reduction in Definition 2 of [MSV'09]
  for (int i = 0; i < kappa; i++)
  {
    m.get_R(ftmp0, kappa, i, expo0);  // R(kappa, i) = ftmp0 * 2^expo0
    ftmp0.abs(ftmp0);                 // |R(kappa, i)| = |ftmp0| * 2^expo0
    expo2 = m.get_row_expo(i);        // R(i, i) = eR[i] * 2^expo2

    FPLLL_DEBUG_CHECK(expo0 == expo1);

    // We want to test if
    //   |R(kappa, i)| <= eta * R(i, i) + theta * R(kappa, kappa)
    //   ftmp0 * 2^expo0 <= eR[i] * 2^expo2 + ftmp1 * 2^expo0, since expo0 == expo1
    //   ftmp0 <= eRr[i] * 2^(expo2 - expo0) + ftmp1
    //   ftmp0 <= ftmp2
    ftmp2.mul_2si(eR[i], expo2 - expo0);
    ftmp2.add(ftmp1, ftmp2);

    if (ftmp0.cmp(ftmp2) > 0)
    {
#ifdef HOUSEHOLDER_DEBUG
      cerr << "This is probably the first time this test is used." << endl;
#endif  // HOUSEHOLDER_DEBUG

      cerr << "Anomaly: weak size reduction is not complete kappa = " << kappa << " and i = " << i
           << endl;

      return false;
    }
  }
#endif  // HOUSEHOLDER_VERIFY_SIZE_REDUCTION_HPLLL

  return true;
}

/*
 * Verify if the basis b inside m is (delta, eta)-hlll reduced.
 * Use a different implementation of the Householder transformation to compute R in this test than
 * the one used to reduced the basis.
 */
template <class ZT, class FT>
int is_hlll_reduced(MatHouseholder<ZT, FT> &m, double delta, double eta, double theta)
{
  int i, j;
  // Temporary variables
  FT ftmp0, ftmp1, ftmp2;
  // FT version of delta and eta
  FT delta_ = delta;
  FT eta_   = eta;
  FT theta_ = theta;

  // Compute the R coefficients of b
  m.update_R_naively();

  // Exponent associated to ftmp0 and ftmp1 (respectively)
  long expo0 = 0;
  long expo1 = 0;
  long expo2 = 0;

  // Verify if |R(j, i)| <= eta * R(i, i) + theta * R(j, j) (weak size-reduction of Definition 2
  // [MSV'09].
  for (j = 0; j < m.get_d(); j++)
  {
    for (i = 0; i < j; i++)
    {
      m.get_R_naively(ftmp0, j, i, expo0);  // R(j, i) = ftmp0 * 2^expo0
      ftmp0.abs(ftmp0);                     // |R(j, i)| = |ftmp0| * 2^expo0
      m.get_R_naively(ftmp1, j, j, expo1);  // R(j, j) = ftmp1 * 2^expo1
      m.get_R_naively(ftmp2, i, i, expo2);  // R(i, i) = ftmp2 * 2^expo2

      FPLLL_DEBUG_CHECK(expo0 == expo1);

      ftmp1.mul(ftmp1, theta_);  // theta_ * R(j, j) = ftmp1 * 2^expo1
      ftmp2.mul(ftmp2, eta_);    // eta_ * R(i, i) = ftmp2 * 2^expo2

      // We want to test if
      //   |R(j, i)| <= eta * R(i, i) + theta * R(j, j)
      //   ftmp0 * 2^expo0 <= ftmp1 * 2^expo0 + ftmp2 * 2^expo2, since expo0 == expo1
      //   ftmp0 <= ftmp1 + ftmp2 * 2^(expo2 - expo0)
      ftmp2.mul_2si(ftmp2, expo2 - expo0);
      ftmp1.add(ftmp1, ftmp2);

      if (ftmp0.cmp(ftmp1) > 0)
        return RED_HLLL_FAILURE;
    }
  }

  // At this step, we verify if two consecutive vectors must be swapped during the hlll-reduction or
  // not (Lovasz's condition)
  for (i = 1; i < m.get_d(); i++)
  {
    m.get_R_naively(ftmp0, i - 1, i - 1, expo0);  // R(i - 1, i - 1) = ftmp0 * 2^expo0
    m.get_R_naively(ftmp1, i, i - 1, expo1);      // R(i, i - 1) = ftmp1 * 2^expo1
    m.get_R_naively(ftmp2, i, i, expo2);          // R(i, i) = ftmp2 * 2^expo2
    FPLLL_DEBUG_CHECK(expo0 == expo1);

    ftmp0.mul(ftmp0, ftmp0);
    ftmp1.mul(ftmp1, ftmp1);
    ftmp2.mul(ftmp2, ftmp2);
    expo0 = 2 * expo0;
    // expo1 = 2 * expo1; : not necessary, since expo0 == expo1
    expo2 = 2 * expo2;
    // Here: R(i - 1, i - 1)^2 = ftmp0 * 2^expo0
    // Here: R(i, i - 1)^2 = ftmp1 * 2^expo1
    // Here: R(i, i)^2 = ftmp2 * 2^expo2

    ftmp0.mul(ftmp0, delta);  // delta * R(i - 1, i - 1)^2 = delta * ftmp0 * 2^expo0

    // We want to test if
    //   delta * R(i - 1, i - 1)^2 <= R(i, i - 1)^2 + R(i, i)^2
    //   ftmp0 * 2^expo0 <= ftmp1 * 2^expo0 + ftmp2 * 2^expo2
    //   ftmp0 <= ftmp1 + ftmp2 * 2^(expo2 - expo0)
    ftmp2.mul_2si(ftmp2, expo2 - expo0);
    ftmp1.add(ftmp1, ftmp2);

    if (ftmp0.cmp(ftmp1) > 0)
      return RED_HLLL_FAILURE;
  }

  return RED_SUCCESS;
}

/** instantiate functions **/

template class HLLLReduction<Z_NR<long>, FP_NR<double>>;
template class HLLLReduction<Z_NR<double>, FP_NR<double>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<double>>;
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<double>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<double>> &m,
                                            double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<long>, FP_NR<double>>(MatHouseholder<Z_NR<long>, FP_NR<double>> &m,
                                           double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<double>, FP_NR<double>>(MatHouseholder<Z_NR<double>, FP_NR<double>> &m,
                                             double delta, double eta, double theta);

#ifdef FPLLL_WITH_LONG_DOUBLE
template class HLLLReduction<Z_NR<long>, FP_NR<long double>>;
template class HLLLReduction<Z_NR<double>, FP_NR<long double>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<long double>>;
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<long double>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<long double>> &m,
                                                 double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<long>, FP_NR<long double>>(MatHouseholder<Z_NR<long>, FP_NR<long double>> &m,
                                                double delta, double eta, double theta);
template int is_hlll_reduced<Z_NR<double>, FP_NR<long double>>(
    MatHouseholder<Z_NR<double>, FP_NR<long double>> &m, double delta, double eta, double theta);
#endif

#ifdef FPLLL_WITH_QD
template class HLLLReduction<Z_NR<long>, FP_NR<dd_real>>;
template class HLLLReduction<Z_NR<double>, FP_NR<dd_real>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<dd_real>>;

template class HLLLReduction<Z_NR<long>, FP_NR<qd_real>>;
template class HLLLReduction<Z_NR<double>, FP_NR<qd_real>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<qd_real>>;
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<qd_real>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<qd_real>> &m,
                                             double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<long>, FP_NR<qd_real>>(MatHouseholder<Z_NR<long>, FP_NR<qd_real>> &m,
                                            double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<double>, FP_NR<qd_real>>(MatHouseholder<Z_NR<double>, FP_NR<qd_real>> &m,
                                              double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<dd_real>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<dd_real>> &m,
                                             double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<long>, FP_NR<dd_real>>(MatHouseholder<Z_NR<long>, FP_NR<dd_real>> &m,
                                            double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<double>, FP_NR<dd_real>>(MatHouseholder<Z_NR<double>, FP_NR<dd_real>> &m,
                                              double delta, double eta, double theta);
#endif

#ifdef FPLLL_WITH_DPE
template class HLLLReduction<Z_NR<long>, FP_NR<dpe_t>>;
template class HLLLReduction<Z_NR<double>, FP_NR<dpe_t>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<dpe_t>>;
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<dpe_t>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<dpe_t>> &m,
                                           double delta, double eta, double theta);
template int is_hlll_reduced<Z_NR<long>, FP_NR<dpe_t>>(MatHouseholder<Z_NR<long>, FP_NR<dpe_t>> &m,
                                                       double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<double>, FP_NR<dpe_t>>(MatHouseholder<Z_NR<double>, FP_NR<dpe_t>> &m,
                                            double delta, double eta, double theta);
#endif

template class HLLLReduction<Z_NR<long>, FP_NR<mpfr_t>>;
template class HLLLReduction<Z_NR<double>, FP_NR<mpfr_t>>;
template class HLLLReduction<Z_NR<mpz_t>, FP_NR<mpfr_t>>;
template int
is_hlll_reduced<Z_NR<mpz_t>, FP_NR<mpfr_t>>(MatHouseholder<Z_NR<mpz_t>, FP_NR<mpfr_t>> &m,
                                            double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<long>, FP_NR<mpfr_t>>(MatHouseholder<Z_NR<long>, FP_NR<mpfr_t>> &m,
                                           double delta, double eta, double theta);
template int
is_hlll_reduced<Z_NR<double>, FP_NR<mpfr_t>>(MatHouseholder<Z_NR<double>, FP_NR<mpfr_t>> &m,
                                             double delta, double eta, double theta);

FPLLL_END_NAMESPACE
