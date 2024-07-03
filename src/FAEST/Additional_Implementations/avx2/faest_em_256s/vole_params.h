#ifndef VOLE_PARMS_H
#define VOLE_PARMS_H

#include <assert.h>

#include "block.h"
#include "prgs.h"
#include "owf_proof.h"
#include "vole_check.h"

// The homomorphic commitments use small field VOLE with a mix of two values of k: VOLE_MIN_K and
// VOLE_MAX_K. k is the number of bits of Delta input to a single VOLE.
#define VOLE_MIN_K (SECURITY_PARAM / BITS_PER_WITNESS)
#define VOLE_MAX_K ((SECURITY_PARAM + BITS_PER_WITNESS - 1) / BITS_PER_WITNESS)

// Number of VOLEs that use VOLE_MIN_K and VOLES_MAX_K.
#define VOLES_MIN_K (BITS_PER_WITNESS - VOLES_MAX_K)
#define VOLES_MAX_K (SECURITY_PARAM % BITS_PER_WITNESS)

static_assert(WITNESS_BITS % 8 == 0, "");
#define WITNESS_BLOCKS ((WITNESS_BITS + 128 * VOLE_BLOCK - 1) / (128 * VOLE_BLOCK))

#define QUICKSILVER_ROW_PAD_TO \
	(128 * VOLE_BLOCK > TRANSPOSE_BITS_ROWS ? 128 * VOLE_BLOCK : TRANSPOSE_BITS_ROWS)

#define QUICKSILVER_ROWS (WITNESS_BITS + SECURITY_PARAM)
#define QUICKSILVER_ROWS_PADDED \
	((QUICKSILVER_ROWS + QUICKSILVER_ROW_PAD_TO - 1) / QUICKSILVER_ROW_PAD_TO) * QUICKSILVER_ROW_PAD_TO

#define VOLE_ROWS (QUICKSILVER_ROWS + VOLE_CHECK_HASH_BYTES * 8)

#define VOLE_COL_BLOCKS ((VOLE_ROWS + 128 * VOLE_BLOCK - 1) / (128 * VOLE_BLOCK))
#define VOLE_COL_STRIDE (VOLE_COL_BLOCKS * 16 * VOLE_BLOCK)
#define VOLE_ROWS_PADDED (VOLE_COL_BLOCKS * 128 * VOLE_BLOCK)

#if defined(PRG_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 256
#define PRG_VOLE_BLOCK_SIZE_SHIFT 1
#else
#define PRG_VOLE_BLOCK_SIZE_SHIFT 0
#endif

// Number of block128s in a prg_vole_block.
#define PRG_VOLE_BLOCK_SIZE (1 << PRG_VOLE_BLOCK_SIZE_SHIFT)
static_assert(PRG_VOLE_BLOCK_SIZE * 16 == sizeof(prg_vole_block), "a `prg_vole_block` must be 16 * PRG_VOLE_BLOCK_SIZE");

// Number of prg_vole_block in a vole_block.
#define PRG_VOLE_BLOCKS (1 << PRG_VOLE_BLOCKS_SHIFT)
#define PRG_VOLE_BLOCKS_SHIFT (VOLE_BLOCK_SHIFT - PRG_VOLE_BLOCK_SIZE_SHIFT)

// VOLE is performed in chunks of VOLE_WIDTH keys, with each column consisting of 1
// vole_block.
#define VOLE_WIDTH (1 << VOLE_WIDTH_SHIFT)
#define VOLE_WIDTH_SHIFT (AES_PREFERRED_WIDTH_SHIFT - PRG_VOLE_BLOCKS_SHIFT)

// Everything aes.h needs from vole_params.h comes before.
#include "aes.h"

#endif
