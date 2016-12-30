/*
 * SimdBuffer.cpp
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#include "SimdBuffer.h"
#include "SimdHelper.h"

__m256i SimdBuffer::EMPTY = _mm256_setzero_si256();

SimdBuffer::SimdBuffer() {

}

SimdBuffer::~SimdBuffer() {

}

__m256i SimdBuffer::FLAG_SHIFT = _mm256_setr_epi32(0, 1, 2, 3, 0, 1, 2, 3);
__m256i SimdBuffer::FLAG_PERMUTE = _mm256_setr_epi32(0, 4, 1, 5, 2, 3, 6, 7);

__m256i SimdBuffer::serve(__m256i input) {
	return input;
}

__m256i SimdBuffer::LOOKUP_SIZE = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2,
		2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

__m256i SimdBuffer::LOOKUP_POS1 = _mm256_setr_epi8(-1, 3, 2, 2, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 0, -1, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0);
__m256i SimdBuffer::LOOKUP_POS2 = _mm256_setr_epi8(-1, -1, -1, 3, -1, 3, 2, 2,
		-1, 3, 2, 2, 1, 1, 1, 1, -1, -1, -1, 3, -1, 3, 2, 2, -1, 3, 2, 2, 1, 1,
		1, 1);
__m256i SimdBuffer::LOOKUP_POS3 = _mm256_setr_epi8(-1, -1, -1, -1, -1, -1, -1,
		3, -1, -1, -1, 3, -1, 3, 2, 2, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1,
		-1, 3, -1, 3, 2, 2);
__m256i SimdBuffer::LOOKUP_POS4 = _mm256_setr_epi8(-1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, 3);
__m256i SimdBuffer::PERMU_POS1 = _mm256_setr_epi32(0, 4, 2, 3, 1, 5, 6, 7);
__m256i SimdBuffer::PERMU_POS2 = _mm256_setr_epi32(5, 0, 2, 3, 4, 1, 6, 7);
__m256i SimdBuffer::PERMU_POS3 = _mm256_setr_epi32(2, 6, 0, 3, 4, 5, 1, 7);
__m256i SimdBuffer::PERMU_POS4 = _mm256_setr_epi32(3, 7, 2, 0, 4, 5, 6, 1);

__m256i SimdBuffer::SHL_POS[5] = { _mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3),
		_mm256_setr_epi32(0, 4, 5, 6, 7, 1, 2, 3), _mm256_setr_epi32(0, 1, 4, 5,
				6, 7, 2, 3), _mm256_setr_epi32(0, 1, 2, 4, 5, 6, 7, 3),
		_mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7) };

__m256i SimdBuffer::SHR_POS[8] = { _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7),
		_mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), _mm256_setr_epi32(6, 7, 0, 1,
				2, 3, 4, 5), _mm256_setr_epi32(5, 6, 7, 0, 1, 2, 3, 4),
		_mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3), _mm256_setr_epi32(3, 4, 5, 6,
				7, 0, 1, 2), _mm256_setr_epi32(2, 3, 4, 5, 6, 7, 0, 1),
		_mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0) };
/**
 * The first two 32-bit integers in input contain the 4-bit block of flag
 */
__m256i SimdBuffer::align(__m256i input, int *size) {
	__m256i flag = _mm256_xor_si256(
			_mm256_cmpeq_epi32(input, SimdHelper::ZERO));
	__m256i sflag = _mm256_sllv_epi32(flag, FLAG_SHIFT);
	flag = _mm256_hadd_epi32(sflag, flag);
	flag = _mm256_hadd_epi32(flag, SimdHelper::ZERO);
	// flag[0] has first 4 bit flag, flag[1] has second 4 bit flag
	flag = _mm256_permutevar8x32_epi32(flag, FLAG_PERMUTE);

	__m256i p1 = _mm256_permutevar8x32_epi32(
			_mm256_shuffle_epi8(LOOKUP_POS1, flag), PERMU_POS1);
	__m256i p2 = _mm256_permutevar8x32_epi32(
			_mm256_shuffle_epi8(LOOKUP_POS2, flag), PERMU_POS2);
	__m256i p3 = _mm256_permutevar8x32_epi32(
			_mm256_shuffle_epi8(LOOKUP_POS3, flag), PERMU_POS3);
	__m256i p4 = _mm256_permutevar8x32_epi32(
			_mm256_shuffle_epi8(LOOKUP_POS4, flag), PERMU_POS4);
	__m256i p1p2 = _mm256_blend_epi32(p1, p2, 0x22);
	__m256i p3p4 = _mm256_blend_epi32(p3, p4, 0x88);
	__m256i allblend = _mm256_blend_epi32(p1p2, p3p4, 0xcc);

	__m256i sizev = _mm256_shuffle_epi8(LOOKUP_SIZE, flag);
	int size1 = _mm256_extract_epi32(size, 0);
	int size2 = _mm256_extract_epi32(size, 1);
	__m256i permute = _mm256_permutevar8x32_epi32(allblend, SHL_POS[*size1]);
	*size = size1 + size2;
	return _mm256_permutevar8x32_epi32(input, permute);
}

__m256i SimdBuffer::shr(__m256i input, int offset) {
	return _mm256_permutevar8x32_epi32(input, SHR_POS[offset]);
}
