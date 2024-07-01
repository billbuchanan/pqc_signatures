#ifndef CONFIG_H
#define CONFIG_H

#define SECURITY_PARAM 192
#define OWF_RIJNDAEL_EVEN_MANSOUR
#define PRG_AES_CTR
#define TREE_PRG_AES_CTR
#define LEAF_PRG_SHAKE
#define BITS_PER_WITNESS 24

// SHA3 is currently the only option.
#define RANDOM_ORACLE_SHA3

// XKCP configuration.
#define XKCP_has_KeccakP1600
#define XKCP_has_KeccakP1600times2
#define XKCP_has_KeccakP1600times4
#define XKCP_has_KeccakP1600times8

#endif
