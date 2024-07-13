#ifndef TRANSPOSE_IMPL_H
#define TRANSPOSE_IMPL_H

#define TRANSPOSE_BITS_ROWS_SHIFT 8

ALWAYS_INLINE void transpose4x4_32(block128* output, const block128* input)
{
	// Notation: inputs rows are lettered a, b, c, d, while the columns are numbered 0, 1, 2, 3.
	// E.g., this makes input[0] be a0a1a2a3.
	block128 a0b0a1b1 = _mm_unpacklo_epi32(input[0], input[1]);
	block128 a2b2a3b3 = _mm_unpackhi_epi32(input[0], input[1]);
	block128 c0d0c1d1 = _mm_unpacklo_epi32(input[2], input[3]);
	block128 c2d2c3d3 = _mm_unpackhi_epi32(input[2], input[3]);
	output[0] = _mm_unpacklo_epi64(a0b0a1b1, c0d0c1d1); // output[0] = a0b0c0d0
	output[1] = _mm_unpackhi_epi64(a0b0a1b1, c0d0c1d1); // output[1] = a1b1c1d1
	output[2] = _mm_unpacklo_epi64(a2b2a3b3, c2d2c3d3); // output[2] = a2b2c2d2
	output[3] = _mm_unpackhi_epi64(a2b2a3b3, c2d2c3d3); // output[3] = a3b3c3d3
}

// Transpose a 4x2 (row manjor) matrix to get a 2x4 matrix. input0 contains the first two rows,
// and input1 has the other two rows.
ALWAYS_INLINE void transpose4x2_32(block128* output, block128 input0, block128 input1)
{
	output[0] = shuffle_2x4xepi32(input0, input1, 0x88); // output[0] = a0b0c0d0
	output[1] = shuffle_2x4xepi32(input0, input1, 0xdd); // output[1] = a1b1c1d1
}

ALWAYS_INLINE block256 transpose2x2_64(block256 input)
{
	return _mm256_permute4x64_epi64(input, 0xd8);
}

ALWAYS_INLINE void transpose2x2_128(block256* output, block256 input0, block256 input1)
{
	block256 a0b0 = _mm256_permute2x128_si256(input0, input1, 0x20);
	block256 a1b1 = _mm256_permute2x128_si256(input0, input1, 0x31);
	output[0] = a0b0;
	output[1] = a1b1;
}

#endif
