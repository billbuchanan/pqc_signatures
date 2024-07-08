/* Copyright (C) 2016 Martin R. Albrecht

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

#include <cstring>
#include <fplll.h>

using namespace std;
using namespace fplll;

template <class FT> int test_str()
{
  int status = 0;
  FT a       = "1.01";
  status |= !(abs(a - 1.01) < 0.001);
  return status;
}

template <class FT> int test_arithmetic()
{
  FT a = 6.0, b = 3.0, c = -2;
  FT d       = 1.0 - (a * b + 2 * c + 2.0) / c;
  int status = 0;
  status |= d.cmp(9);
  d += 1.0;
  status |= d.cmp(10);
  return status;
}

template <class FT> bool test_hypot()
{
  FT a, b, c, d;
  a = 3.0;
  b = 4.0;
  // the hypot function is asymmetric,
  // so we test both hypot(a,b) and hypot(b,a)
  // a^2 + b^2 = c^2 = d^2 = 5^2
  c.hypot(a, b);
  d.hypot(b, a);
  int status = (!(abs(c - 5.0) < 0.001) || !(abs(d - 5.0) < 0.001));

  if (status == 1)
  {
    cerr << c;
  }
  return status;
}

/**
   @brief

   zero on success.

   @return
*/

template <class FT> int test_std()
{
  FT a;
  a          = 6.1;
  int status = 0;
  status |= !(abs(sqrt(a) - std::sqrt(a.get_d())) < 0.001);
  status |= !(abs(log(a) - std::log(a.get_d())) < 0.001);
  status |= !(abs(exp(a) - std::exp(a.get_d())) < 0.001);
  status |= !(abs(floor(a) - std::floor(a.get_d())) < 0.001);
  status |= !(abs(pow_si(a, 2) - std::pow(a.get_d(), 2.0)) < 0.001);
  return status;
}

template <class FT> int test_root()
{
  FT a;
  a          = 6.1;
  int status = !(abs(root(a, 3.0) - std::pow(a.get_d(), 1 / 3.0)) < 0.001);
  return status;
}

int main()
{

  int status = 0;
  status |= test_arithmetic<FP_NR<double>>();
#ifdef FPLLL_WITH_LONG_DOUBLE
  status |= test_arithmetic<FP_NR<long double>>();
#endif
#ifdef FPLLL_WITH_DPE
  status |= test_arithmetic<FP_NR<dpe_t>>();
#endif
#ifdef FPLLL_WITH_QD
  status |= test_arithmetic<FP_NR<dd_real>>();
  status |= test_arithmetic<FP_NR<qd_real>>();
#endif
  status |= test_arithmetic<FP_NR<mpfr_t>>();

  status |= test_std<FP_NR<double>>();
#ifdef FPLLL_WITH_LONG_DOUBLE
  status |= test_std<FP_NR<long double>>();
#endif
#ifdef FPLLL_WITH_DPE
  status |= test_std<FP_NR<dpe_t>>();
#endif
#ifdef FPLLL_WITH_QD
  status |= test_std<FP_NR<dd_real>>();
  status |= test_std<FP_NR<qd_real>>();
#endif
  status |= test_std<FP_NR<mpfr_t>>();

  status |= test_root<FP_NR<double>>();
#ifdef FPLLL_WITH_LONG_DOUBLE
  status |= test_root<FP_NR<long double>>();
#endif
#ifdef FPLLL_WITH_QD
  status |= test_root<FP_NR<dd_real>>();
  status |= test_root<FP_NR<qd_real>>();
#endif
  status |= test_root<FP_NR<mpfr_t>>();

  status |= test_str<FP_NR<double>>();
#ifdef FPLLL_WITH_LONG_DOUBLE
  status |= test_str<FP_NR<long double>>();
#endif
#ifdef FPLLL_WITH_DPE
  status |= test_str<FP_NR<dpe_t>>();
#endif
#ifdef FPLLL_WITH_QD
  status |= test_str<FP_NR<dd_real>>();
  status |= test_str<FP_NR<qd_real>>();
#endif
  status |= test_str<FP_NR<mpfr_t>>();

  status |= test_hypot<FP_NR<double>>();
#ifdef FPLLL_WITH_LONG_DOUBLE
  status |= test_hypot<FP_NR<long double>>();
#endif

#ifdef FPLLL_WITH_DPE
  status |= test_hypot<FP_NR<dpe_t>>();
#endif

#ifdef FPLLL_WITH_QD
  status |= test_hypot<FP_NR<dd_real>>();
  status |= test_hypot<FP_NR<qd_real>>();
#endif
  status |= test_hypot<FP_NR<mpfr_t>>();

  if (status == 0)
  {
    cerr << "All tests passed." << endl;
    return 0;
  }
  else
  {
    return -1;
  }

  return 0;
}
