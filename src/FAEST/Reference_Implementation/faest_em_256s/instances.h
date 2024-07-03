/*
 *  SPDX-License-Identifier: MIT
 */

#ifndef INSTANCES_H
#define INSTANCES_H

#include "macros.h"

#include <stddef.h>
#include <stdint.h>
FAEST_BEGIN_C_DECL

#define MAX_LAMBDA 256
#define MAX_LAMBDA_BYTES (MAX_LAMBDA / 8)
#define MAX_DEPTH 12

typedef enum faest_paramid_t {
  PARAMETER_SET_INVALID   = 0,
  FAEST_128S              = 1,
  FAEST_128F              = 2,
  FAEST_192S              = 3,
  FAEST_192F              = 4,
  FAEST_256S              = 5,
  FAEST_256F              = 6,
  FAEST_EM_128S           = 7,
  FAEST_EM_128F           = 8,
  FAEST_EM_192S           = 9,
  FAEST_EM_192F           = 10,
  FAEST_EM_256S           = 11,
  FAEST_EM_256F           = 12,
  PARAMETER_SET_MAX_INDEX = 13
} faest_paramid_t;

typedef struct faest_param_t {
  uint32_t lambda;
  uint32_t Nwd;
  uint32_t Ske;
  uint32_t R;
  uint32_t Senc;
  uint32_t beta;
  uint32_t l;
  uint32_t Lke;
  uint32_t Lenc;
  uint32_t c;
  uint32_t tau;
  uint32_t k0;
  uint32_t k1;
  uint32_t t0;
  uint32_t t1;
  uint32_t b;
  uint32_t sigSize;
  uint32_t pkSize;
  uint32_t skSize;
} faest_param_t;

// TODO - Add paramset struct paramset_t here, containing numround, numsbox, seedSizeBytes,
// digestSizeBytes.... ?
typedef struct faest_paramset_t {
  faest_param_t faest_param;
  faest_paramid_t faest_paramid;
} faest_paramset_t;

const char* faest_get_param_name(faest_paramid_t paramid);
int faest_check_paramset(faest_paramset_t* paramset);
faest_paramset_t faest_get_paramset(faest_paramid_t paramid);

FAEST_END_C_DECL

#endif
