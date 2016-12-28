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

__m256i serve(__m256i input) {
	__m256i flag = _mm256_xor_si256(
			_mm256_cmpeq_epi32(input, SimdHelper::ZERO));
	flag = _mm256_sllv_epi32(flag, SHIFT);
	flag = _mm256_hadd_epi32(flag, flag);
	flag = _mm256_hadd_epi32(flag, flag);
	// flag[0] has first 4 bit flag, flag[4] has second 4 bit flag


}
