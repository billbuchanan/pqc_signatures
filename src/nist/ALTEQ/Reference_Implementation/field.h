#ifndef FIELD_H_
#define FIELD_H_

#include "api.h"

extern uint64_t multiplicationModuloP(const uint64_t a, const uint64_t b);

extern uint64_t reductionModuloP(const uint64_t a);

extern uint64_t reductionStrict(const uint64_t a);

extern void setInversionModuloP(uint64_t *set);

#endif
