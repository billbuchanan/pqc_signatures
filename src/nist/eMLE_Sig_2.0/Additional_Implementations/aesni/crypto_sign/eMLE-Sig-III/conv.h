#ifndef _CONV_H
#define _CONV_H

#include <stdint.h>

void conv_64_mod_5(int64_t *out, const int64_t *a, const int64_t *b);
void conv_96_mod_5(int64_t *out, const int64_t *a, const int64_t *b);
void conv_128_mod_5(int64_t *out, const int64_t *a, const int64_t *b);

void conv_64_mod_557(int64_t *out, const int64_t *a, const int64_t *b);
void conv_96_mod_823(int64_t *out, const int64_t *a, const int64_t *b);
void conv_128_mod_1097(int64_t *out, const int64_t *a, const int64_t *b);

void conv_64_mod_67108864(int64_t *out, const int64_t *a, const int64_t *b);
void conv_96_mod_268435456(int64_t *out, const int64_t *a, const int64_t *b);
void conv_128_mod_1073741824(int64_t *out, const int64_t *a, const int64_t *b);

void conv_64(int64_t *out, const int64_t *a, const int64_t *b);
void conv_96(int64_t *out, const int64_t *a, const int64_t *b);
void conv_128(int64_t *out, const int64_t *a, const int64_t *b);

void add_64_mod_5(int64_t *out, const int64_t *a, const int64_t *b);
void add_64_mod_557(int64_t *out, const int64_t *a, const int64_t *b);
void add_64_mod_67108864(int64_t *out, const int64_t *a, const int64_t *b);

void add_96_mod_5(int64_t *out, const int64_t *a, const int64_t *b);
void add_96_mod_823(int64_t *out, const int64_t *a, const int64_t *b);
void add_96_mod_268435456(int64_t *out, const int64_t *a, const int64_t *b);

void add_128_mod_5(int64_t *out, const int64_t *a, const int64_t *b);
void add_128_mod_1097(int64_t *out, const int64_t *a, const int64_t *b);
void add_128_mod_1073741824(int64_t *out, const int64_t *a, const int64_t *b);

#endif
