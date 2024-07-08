#ifndef PARAMS_H
#define PARAMS_H

#define PARAM_M 		736 //should div 16, res is 0
#define PARAM_N			848 
#define PARAM_p			4096
#define PARAM_q			16
#define PARAM_Q			65536
#define PARAM_SIGMAG	PARAM_q * PARAM_R
#define PARAM_SIGMA1	1055.6  // std of sig1 
#define PARAM_TRUNC		10556   // 10 sigma /////////////////////
#define PARAM_BOUND		62521.3
#define PARAM_BOUND_SQUARE	3908912953.69
#define PARAM_QBITS		16
#define PARAM_SIG_BYTES 2495

#define PARAMS_STRIPE_STEP  8

#define PARAM_R				1.3250518175969843
#define PARAM_L22_BYTES		PARAM_N * (1 + PARAM_N) / 2 * 8
#define PARAM_L32_BYTES		PARAM_M * PARAM_N * 8
#define PARAM_L33_BYTES		PARAM_M * (1 + PARAM_M) / 2 * 8

#define PARAM_SEED_BYTES	32
#define PARAM_R_BYTES		2 * (PARAM_N + PARAM_M) * PARAM_M / 8
#define PARAM_B_BYTES		PARAM_QBITS * PARAM_M * PARAM_M / 8
#define PARAM_PK_BYTES		(PARAM_B_BYTES + PARAM_SEED_BYTES)
#define PARAM_SK_BYTES		(PARAM_R_BYTES + PARAM_L22_BYTES + PARAM_L32_BYTES + PARAM_L33_BYTES)

#define PI					3.141592653589793
#define PARAM_SALT_BYTES	40


#define SCALE_BITS 12
#define SCALE (1u << SCALE_BITS)

#define PARAM_SIG_SIZE (PARAM_M+PARAM_N)
#define PARAM_SIG_ENCODE_MAX_BYTES (PARAM_SIG_HIGHBIT_ENCODE_MAX_BYTES+PARAM_SIG_LOWBIT_ENCODE_BYTES)
#define PARAM_SIG_HIGHBIT_ENCODE_MAX_BYTES ((PARAM_SIG_SIZE*SCALE_BITS)/8+4)
#define PARAM_SIG_LOWBIT_ENCODE_BYTES (PARAM_SIG_SIZE)
#define PARAM_COMPRESSED_SIG_BYTES (PARAM_SIG_ENCODE_MAX_BYTES + 2 + PARAM_SALT_BYTES)


typedef long long 				ll;
typedef unsigned long long 		ull;

#define mat_element(M, nb_col, i, j) 	M[((i)*(nb_col)) + (j)]
#define tri_mat_element(M, i, j) 		M[(i)*((i)+1)/2 + (j)]


#endif
