/*
 * SimdVecBuffer.cpp
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#include "SimdVecBuffer.h"

#include <stdlib.h>
#include "../simd/SimdHelper.h"

namespace SimdVecBufferConstants {
__m256i FLAG_SHIFT = _mm256_setr_epi32(3, 2, 1, 0, 3, 2, 1, 0);
__m256i FLAG_PERMUTE = _mm256_setr_epi32(0, 4, 1, 5, 2, 3, 6, 7);

__m256i LOOKUP_SIZE = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3,
		3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

__m256i LOOKUP_POS1 = _mm256_setr_epi8(-1, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, -1, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4);
__m256i LOOKUP_POS2 = _mm256_setr_epi8(-1, -1, -1, 3, -1, 3, 2, 2, -1, 3, 2, 2,
		1, 1, 1, 1, -1, -1, -1, 7, -1, 7, 6, 6, -1, 7, 6, 6, 5, 5, 5, 5);
__m256i LOOKUP_POS3 = _mm256_setr_epi8(-1, -1, -1, -1, -1, -1, -1, 3, -1, -1,
		-1, 3, -1, 3, 2, 2, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1, 7, -1, 7,
		6, 6);
__m256i LOOKUP_POS4 = _mm256_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 7);
__m256i PERMU_POS1 = _mm256_setr_epi32(0, 4, 2, 3, 1, 5, 6, 7);
__m256i PERMU_POS2 = _mm256_setr_epi32(5, 0, 2, 3, 4, 1, 6, 7);
__m256i PERMU_POS3 = _mm256_setr_epi32(2, 6, 0, 3, 4, 5, 1, 7);
__m256i PERMU_POS4 = _mm256_setr_epi32(3, 7, 2, 0, 4, 5, 6, 1);

__m256i SHL128_POS[5] = { _mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3),
		_mm256_setr_epi32(0, 4, 5, 6, 7, 1, 2, 3), _mm256_setr_epi32(0, 1, 4, 5,
				6, 7, 2, 3), _mm256_setr_epi32(0, 1, 2, 4, 5, 6, 7, 3),
		_mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7) };

__m256i SHL_POS[8] = { _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7),
		_mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0), _mm256_setr_epi32(2, 3, 4, 5,
				6, 7, 0, 1), _mm256_setr_epi32(3, 4, 5, 6, 7, 0, 1, 2),
		_mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3), _mm256_setr_epi32(5, 6, 7, 0,
				1, 2, 3, 4), _mm256_setr_epi32(6, 7, 0, 1, 2, 3, 4, 5),
		_mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6) };

__m256i SHR_POS[8] = { _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7),
		_mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), _mm256_setr_epi32(6, 7, 0, 1,
				2, 3, 4, 5), _mm256_setr_epi32(5, 6, 7, 0, 1, 2, 3, 4),
		_mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3), _mm256_setr_epi32(3, 4, 5, 6,
				7, 0, 1, 2), _mm256_setr_epi32(2, 3, 4, 5, 6, 7, 0, 1),
		_mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0) };

__m256i blend_0(__m256i a, __m256i b) {
	return b;
}
__m256i blend_1(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xfe);
}
__m256i blend_2(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xfc);
}
__m256i blend_3(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xf8);
}
__m256i blend_4(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xf0);
}
__m256i blend_5(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xe0);
}
__m256i blend_6(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xc0);
}
__m256i blend_7(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0x80);
}

__m256i (*BLEND[8])(__m256i, __m256i
		)= {blend_0,blend_1,blend_2,blend_3,blend_4,blend_5,blend_6,blend_7};

}

using namespace SimdVecBufferConstants;

__m256i SimdVecBuffer::EMPTY = _mm256_setzero_si256();

SimdVecBuffer::SimdVecBuffer() {
	buffer = EMPTY;
	bufferSize = 0;
}

SimdVecBuffer::~SimdVecBuffer() {

}

void* SimdVecBuffer::operator new(size_t num) {
	return aligned_alloc(32, num);
}

__m256i SimdVecBuffer::serve(__m256i input, int* outputSize) {
	int inputSize;
	__m256i pattern;
	__m256i aligned = align(input, &inputSize, &pattern);
	if (inputSize == 0) {
		*outputSize = 0;
		return EMPTY;
	}
	__m256i oldBuffer = buffer;
	int oldBufferSize = bufferSize;

	// update old buffer
	if (inputSize + oldBufferSize > 8) {
		bufferSize = inputSize + oldBufferSize - 8;
		buffer = shl(aligned, inputSize - bufferSize);

		*outputSize = 8;
		return merge(oldBuffer, aligned, oldBufferSize);
	} else if (inputSize + oldBufferSize == 8) {
		bufferSize = 0;

		*outputSize = 8;
		return merge(oldBuffer, aligned, oldBufferSize);
	} else {
		bufferSize = inputSize + oldBufferSize;
		buffer = merge(oldBuffer, aligned, oldBufferSize);

		*outputSize = 0;
		return EMPTY;
	}

	return aligned;
}

__m256i SimdVecBuffer::purge(int* outputSize) {
	*outputSize = bufferSize;
	bufferSize = 0;
	return buffer;
}

/**
 * The first two 32-bit integers in input contain the 4-bit block of flag
 */
__m256i SimdVecBuffer::align(__m256i input, int *size, __m256i* pattern) {
	__m256i flag = _mm256_add_epi32(_mm256_cmpeq_epi32(input, SimdHelper::ZERO),
	SimdHelper::ONE);
	if(_mm256_testz_si256(flag, SimdHelper::MAX)) {
		*pattern = SHL_POS[0];
		*size = 0;
		return input;
	}
	flag = _mm256_sllv_epi32(flag, FLAG_SHIFT);
	flag = _mm256_hadd_epi32(flag, flag);
	flag = _mm256_hadd_epi32(flag, flag);
	// flag[0-3] has first 4 bit flag, flag[4-7] has second 4 bit flag

	__m256i sizev = _mm256_shuffle_epi8(LOOKUP_SIZE, flag);
	int size1 = _mm256_extract_epi32(sizev, 0);
	int size2 = _mm256_extract_epi32(sizev, 4);
	*size = size1 + size2;

	__m256i p1 = _mm256_shuffle_epi8(LOOKUP_POS1, flag);
	__m256i p2 = _mm256_shuffle_epi8(LOOKUP_POS2, flag);
	__m256i p3 = _mm256_shuffle_epi8(LOOKUP_POS3, flag);
	__m256i p4 = _mm256_shuffle_epi8(LOOKUP_POS4, flag);
	__m256i p1p2 = _mm256_blend_epi32(p1, p2, 0x22);
	__m256i p3p4 = _mm256_blend_epi32(p3, p4, 0x88);
	__m256i allblend = _mm256_blend_epi32(p1p2, p3p4, 0xcc);

	__m256i permute = _mm256_permutevar8x32_epi32(allblend, SHL128_POS[size1]);

	*pattern = permute;

	return _mm256_permutevar8x32_epi32(input, permute);
}

__m256i SimdVecBuffer::shl(__m256i input, int offset) {
	return _mm256_permutevar8x32_epi32(input, SHL_POS[offset]);
}

__m256i SimdVecBuffer::shr(__m256i input, int offset) {
	return _mm256_permutevar8x32_epi32(input, SHR_POS[offset]);
}

__m256i SimdVecBuffer::merge(__m256i a, __m256i b, int sizea) {
	return BLEND[sizea](a, shr(b, sizea));
}

