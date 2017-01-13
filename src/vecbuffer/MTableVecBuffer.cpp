/*
 * MTableVecBuffer.cpp
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#include "MTableVecBuffer.h"

#include <stdlib.h>
#include "../simd/SimdHelper.h"
#include "../lookup/LookupHelper.h"

namespace MTableVecBufferConstants {

int SIZE[256];

__m256i* createPermuTable() {

	__m256i* memory = (__m256i*)aligned_alloc(32,256*sizeof(__m256i));

	int buffer[8];
	for(int i = 0; i < 256;i++) {
		int bfcnt = 0;
		for(int j = 0; j < 8;j++) {
			if(i & (1<<j)) {
				buffer[bfcnt++] = j;
			}
		}
		for(;bfcnt < 8; bfcnt++) {
			buffer[bfcnt] = -1;
		}
		SIZE[i] = popcount(i);
		memory[i] = _mm256_setr_epi32(buffer[0], buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6], buffer[7]);
	}

	return memory;
}

__m256i* PERMU = createPermuTable();

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

using namespace MTableVecBufferConstants;

__m256i MTableVecBuffer::EMPTY = _mm256_setzero_si256();

MTableVecBuffer::MTableVecBuffer() {
	buffer = EMPTY;
	bufferSize = 0;
}

MTableVecBuffer::~MTableVecBuffer() {

}

void* MTableVecBuffer::operator new(size_t num) {
	return aligned_alloc(32, num);
}

__m256i MTableVecBuffer::serve(__m256i input, int* outputSize) {
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

__m256i MTableVecBuffer::purge(int* outputSize) {
	*outputSize = bufferSize;
	bufferSize = 0;
	return buffer;
}

/**
 * The first two 32-bit integers in input contain the 4-bit block of flag
 */
__m256i MTableVecBuffer::align(__m256i input, int *size, __m256i* pattern) {
	__m256i flag = _mm256_cmpeq_epi32(input, SimdHelper::ZERO);

	if(_mm256_testz_si256(flag, SimdHelper::MAX)) {
		*pattern = SHL_POS[0];
		*size = 0;
		return input;
	}

	int index = _mm256_movemask_ps(_mm256_castsi256_ps(flag));
	__m256i permute = PERMU[index];

	return _mm256_permutevar8x32_epi32(input, permute);
}

__m256i MTableVecBuffer::shl(__m256i input, int offset) {
	return _mm256_permutevar8x32_epi32(input, SHL_POS[offset]);
}

__m256i MTableVecBuffer::shr(__m256i input, int offset) {
	return _mm256_permutevar8x32_epi32(input, SHR_POS[offset]);
}

__m256i MTableVecBuffer::merge(__m256i a, __m256i b, int sizea) {
	return BLEND[sizea](a, shr(b, sizea));
}

