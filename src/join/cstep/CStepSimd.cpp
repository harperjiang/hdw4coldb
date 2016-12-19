/*
 * CStepSimd.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStepSimd.h"
#include <x86intrin.h>

CStepSimd::CStepSimd() {
	// TODO Auto-generated constructor stub

}

CStepSimd::~CStepSimd() {
	// TODO Auto-generated destructor stub
}

virtual uint CStepSimd::filter(uint* key, uint keylength, ulong* bitmap,
		uint bitmapSize, uint* gathered) {

	__m256i hashFactor = _mm256_set1_epi32((int) UINT32_C(2654435761));
	__m256i bitmapSize = _mm256_set1_epi32((int) bitmapSize);
	__m256i byteSize = _mm256_set1_epi32(32);

	for (uint i = 0; i < keylength / 8; i++) {
		__m256i loadkey = _mm256_load_si256(key + i * 8);

		__m256i hashed = _mm256_irem_epi32(
				_mm256_mullo_epi32(loadkey, hashFactory), bitmapSize);
		__m256i offset;
		__m256i index = _mm256_idivrem_epi32(&offset, hashed, byteSize);

		// Use index to load from bitmap
		__m256i byte = _mm256_i32gather_epi32(bitmap, index, 2);
		// Use offset to load from pattern
		__m256i ptn = _mm256_i32gather_epi32(pattern, offset, 1);

		__m256i result = _mm256_and_si256(byte, ptn);
	}
	if (keylength % 8) {

	}

	return 0;
}
