#ifndef FAEST_VC_H
#define FAEST_VC_H

#include <stdint.h>

#include "instances.h"
#include "utils.h"

FAEST_BEGIN_C_DECL

typedef struct vec_com_t {
  uint8_t* h;
  uint8_t* k;
  uint8_t* com;
  uint8_t* sd;
} vec_com_t;

typedef struct vec_com_rec_t {
  uint8_t* h;
  uint8_t* k;
  uint8_t* com;
  uint8_t* s;
} vec_com_rec_t;

uint64_t getBinaryTreeNodeCount(uint32_t numVoleInstances);
uint64_t getNodeIndex(uint64_t depth, uint64_t levelIndex);

int BitDec(uint32_t leafIndex, uint32_t depth, uint8_t* out);
uint64_t NumRec(uint32_t depth, const uint8_t* bi);

void vector_commitment(const uint8_t* rootKey, const uint8_t* iv, const faest_paramset_t* params,
                       uint32_t lambda, uint32_t lambdaBytes, vec_com_t* vecCom,
                       uint32_t numVoleInstances);

void vector_open(const uint8_t* k, const uint8_t* com, const uint8_t* b, uint8_t* pdec,
                 uint8_t* com_j, uint32_t numVoleInstances, uint32_t lambdaBytes);

void vector_reconstruction(const uint8_t* iv, const uint8_t* pdec, const uint8_t* com_j,
                           const uint8_t* b, uint32_t lambda, uint32_t lambdaBytes,
                           uint32_t numVoleInstances, uint32_t depth, vec_com_rec_t* vecComRec);

int vector_verify(const uint8_t* iv, const uint8_t* pdec, const uint8_t* com_j, const uint8_t* b,
                  uint32_t lambda, uint32_t lambdaBytes, uint32_t numVoleInstances, uint32_t depth,
                  vec_com_rec_t* rec, const uint8_t* vecComH);

void vec_com_clear(vec_com_t* com);
void vec_com_rec_clear(vec_com_rec_t* rec);

FAEST_END_C_DECL

#endif
