#ifndef PARAMS_H
#define PARAMS_H

#define MEDS_name "toy"

#define MEDS_digest_bytes 16
#define MEDS_pub_seed_bytes 32
#define MEDS_sec_seed_bytes 32
#define MEDS_st_seed_bytes 16
#define MEDS_st_salt_bytes 32

#define MEDS_p 8191
#define GFq_t uint16_t
#define GFq_bits 13
#define GFq_bytes 2

#define MEDS_m 10
#define MEDS_n 10
#define MEDS_k 10

#define MEDS_s 4
#define MEDS_t 16
#define MEDS_w 6

#define MEDS_seed_tree_height 4
#define SEED_TREE_size 31
#define MEDS_max_path_len 8

#define MEDS_t_mask 0x0000000F
#define MEDS_t_bytes 1

#define MEDS_s_mask 0x00000003

#define MEDS_PK_BYTES 3593
#define MEDS_SK_BYTES 1042
#define MEDS_SIG_BYTES 2132

#endif

