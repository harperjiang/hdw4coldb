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

__m256i SHIFT = _mm256_setr_epi32(0, 1, 2, 3, 0, 1, 2, 3);
__m256i FLAG_PERMUTE = _mm256_setr_epi32(0, 4, 1, 5, 2, 3, 6, 7);

__m256i serve(__m256i input) {
	__m256i flag = _mm256_xor_si256(
			_mm256_cmpeq_epi32(input, SimdHelper::ZERO));
	__m256i sflag = _mm256_sllv_epi32(flag, SHIFT);
	flag = _mm256_hadd_epi32(sflag, flag);
	flag = _mm256_hadd_epi32(flag, SimdHelper::ZERO);
	flag = _mm256_permutevar8x32_epi32(flag, FLAG_PERMUTE);
	// flag[0] has first 4 bit flag, flag[1] has second 4 bit flag
	__m256i ncvec = _mm256_hadd_epi32(flag, SimdHelper::ZERO);
	// numCount[1] has the number of count
	int numCount = _mm256_extract_epi32(ncvec,1);
	if(numCount > 4) {
		// Already dense
		return input;
	}

}



