/*
 * Copyright 2016-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

// shrinked version of openssl-3.1.0/include/internal/refcount.h

#pragma once

#include <openssl/e_os2.h>
#include <openssl/trace.h>

#if defined(OPENSSL_THREADS) && !defined(OPENSSL_DEV_NO_ATOMICS)

#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L \
      && !defined(__STDC_NO_ATOMICS__)
#   include <stdatomic.h>
#  endif

#  if defined(HAVE_C11_ATOMICS) && defined(ATOMIC_INT_LOCK_FREE) \
      && ATOMIC_INT_LOCK_FREE > 0

typedef _Atomic int CRYPTO_REF_COUNT;

#  elif defined(__GNUC__) && defined(__ATOMIC_RELAXED) && __GCC_ATOMIC_INT_LOCK_FREE > 0

typedef int CRYPTO_REF_COUNT;

#  elif defined(__ICL) && defined(_WIN32)

typedef volatile int CRYPTO_REF_COUNT;

#  elif defined(_MSC_VER) && _MSC_VER>=1200

typedef volatile int CRYPTO_REF_COUNT;

#  endif
#else

typedef int CRYPTO_REF_COUNT;

#endif
