#ifndef COEFF_H
#define COEFF_H

#include <stdint.h>
#include <stddef.h>
#include "params.h"

typedef uint16_t coeff;
typedef int16_t s_coeff;
typedef uint32_t d_coeff;
typedef int32_t s_d_coeff;
typedef uint64_t q_coeff;
typedef int64_t s_q_coeff;

#define D_COEFF_SIZE 32
#define Q_COEFF_SIZE 64

#define print_coeff FULEECA_NAMESPACE(_print_coeff)
void print_coeff(const coeff *p, const size_t n);

#define print_s_coeff FULEECA_NAMESPACE(_print_s_coeff)
void print_s_coeff(const s_coeff *p, const size_t n);

#define print_s_d_coeff FULEECA_NAMESPACE(_print_s_d_coeff)
void print_s_d_coeff(const s_d_coeff *p, const size_t n);

#define coeff_init_zero FULEECA_NAMESPACE(_coeff_init_zero)
void coeff_init_zero(coeff *p, const size_t n);

#define coeff_red_modp FULEECA_NAMESPACE(_coeff_red_modp)
coeff coeff_red_modp(const d_coeff a);

#define coeff_add_modp FULEECA_NAMESPACE(_coeff_add_modp)
coeff coeff_add_modp(const coeff a, const coeff b);

#define coeff_sub_modp FULEECA_NAMESPACE(_coeff_sub_modp)
coeff coeff_sub_modp(const coeff a, const coeff b);

#define coeff_mul_modp FULEECA_NAMESPACE(_coeff_mul_modp)
coeff coeff_mul_modp(const coeff a, const coeff b);

#define coeff_lee_weight FULEECA_NAMESPACE(_coeff_lee_weight)
coeff coeff_lee_weight(const coeff a);

#define s_coeff_lee_weight FULEECA_NAMESPACE(_s_coeff_lee_weight)
coeff s_coeff_lee_weight(const s_coeff a);

#define s_d_coeff_lee_weight FULEECA_NAMESPACE(_s_d_coeff_lee_weight)
d_coeff s_d_coeff_lee_weight(const s_d_coeff a);

#endif
