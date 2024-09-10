#ifndef PARAMETERS_H
#define PARAMETERS_H

#define PARAM_SECURITY 256
#define PARAM_LABEL "mqom-31-L5"

// SD Parameters
#define PARAM_FIELD_SIZE 31
#define PARAM_q PARAM_FIELD_SIZE
#define PARAM_n 106
#define PARAM_m 106
#define PARAM_MATRIX_BYTESIZE (((((PARAM_n*(PARAM_n+1))>>1)+127)>>7)<<7)

// MPC Parameters
#define PARAM_n1 6
#define PARAM_n2 18
#define PARAM_last 4 /* n - m1*(m2-1) */
#define PARAM_eta 10
#define PARAM_EXT_DEGREE PARAM_eta

// MPCitH Parameters
#define PARAM_NB_EXECUTIONS 42
#define PARAM_NB_PARTIES 256
#define PARAM_LOG_NB_PARTIES 8

// Signature Parameters
#define PARAM_SEED_SIZE (256/8)
#define PARAM_SALT_SIZE (512/8)
#define PARAM_DIGEST_SIZE (512/8)

// Hash Domain Separation
#define HASH_PREFIX_COMMITMENT 0
#define HASH_PREFIX_FIRST_CHALLENGE 1
#define HASH_PREFIX_SECOND_CHALLENGE 2
#define HASH_PREFIX_THIRD_CHALLENGE 3
#define HASH_PREFIX_SEED_TREE 4

#endif /* PARAMETERS_H */
