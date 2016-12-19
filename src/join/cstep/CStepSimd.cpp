/*
 * CStepSimd.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStepSimd.h"
#include <immintrin.h>
#include <stdlib.h>

__m256i remainder_epi32(__m256i a, uint b) {
	uint* as = (uint*) &a;
	return _mm256_setr_epi32(as[0] % b, as[1] % b, as[2] % b, as[3] % b,
			as[4] % b, as[5] % b, as[6] % b, as[7] % b);
}

__m256i divrem_epi32(__m256i* remainder, __m256i a, uint b) {
	uint* rem = (uint*)remainder;
	uint* as = (uint*)&a;
	for(uint i = 0; i < 8;i++) {
		rem[i] = as[i]%b;
	}
	return _mm256_setr_epi32(as[0] / b, as[1] / b, as[2] / b, as[3] / b,
	as[4] / b, as[5] / b, as[6] / b, as[7] / b);
}

CStepSimd::CStepSimd() {

}

CStepSimd::~CStepSimd() {
	if (NULL != probe)
		::free(probe);
	probe = NULL;
}

void CStepSimd::buildProbe(kvlist* inner) {
	this->probeSize = inner->size;
	this->probe = (uint*) ::aligned_alloc(32, sizeof(uint) * probeSize);
	for (uint i = 0; i < probeSize; i++) {
		probe[i] = inner->entries[i].key;
	}
}

void CStepSimd::init() {

	hashFactor = _mm256_set1_epi32((int) UINT32_C(2654435761));

	// Initialize pattern
	pattern = new uint[32];
	for (uint i = 0; i < 32; i++) {
		pattern[i] = UINT32_C(1) << i;
	}
}

__m256i CStepSimd::check_bitmap(__m256i input) {
	uint bitmapSize = _lookup->bitmap_size;
	uint byteSize = 32;

	__m256i hashed = remainder_epi32(_mm256_mullo_epi32(input, hashFactor),
			bitmapSize);
	__m256i offset;
	__m256i index = divrem_epi32(&offset, hashed, byteSize);

	// Use index to load from bitmap
	__m256i byte = _mm256_i32gather_epi32((int* )_lookup->bitmap, index, 2);
	// Use offset to load from pattern
	__m256i ptn = _mm256_i32gather_epi32(pattern, offset, 1);

	// non-zero for selected key, zero for abandoned key
	__m256i selector = _mm256_and_si256(byte, ptn);
}

uint CStepSimd::filter(uint* gathered) {

	for (uint i = 0; i < probeSize / 8; i++) {
		__m256i loadkey = _mm256_load_si256((__m256i *) (probe + i * 8));
		check_bitmap(loadkey);
	}
	if (probeSize % 8) {
		uint psize = probeSize % 8;
		uint* start = probe + (probeSize / 8) * 8;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		check_bitmap(loadpartial);
	}

	return 0;
}

uint CStepSimd::lookup(uint* key, uint keylength) {
	return 0;
}
