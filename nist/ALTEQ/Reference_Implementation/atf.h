#ifndef ATF_H_
#define ATF_H_

#include "api.h"
#include "field.h"
#include<mm_malloc.h>

extern void actingOnATFS(uint64_t *atf_out, const uint64_t *atf_in, const uint64_t *columns, const int n_atf);

extern void invertingOnATF(uint64_t *atf_out, const uint64_t *atf_in, const uint64_t *columns);

extern void actingOnATFSwTensor(uint64_t *atf0, uint64_t *atf1, uint64_t* mat);

#endif

