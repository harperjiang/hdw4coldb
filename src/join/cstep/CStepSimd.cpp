/*
 * CStepSimd.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStepSimd.h"
#include <x86intrin.h>
#include <stdlib.h>

#include "../../simd/SimdHelper.h"

#define BITMAP_UNIT 32

__m256i CStepSimd::HASH_FACTOR = _mm256_set1_epi32((int) UINT32_C(2654435761));
__m256i CStepSimd::ZERO = _mm256_setzero_si256();
__m256i CStepSimd::ONE = _mm256_set1_epi32(1);
__m256i CStepSimd::TWO = _mm256_set1_epi32(2);
__m256i CStepSimd::MAX = _mm256_set1_epi32(-1);

/**
 * Check 64-bit bitmap to see if the given key exists in the bitmap
 */
__m256i CStepSimd::check_bitmap(ulong* bitmap, uint bitmapSize, __m256i input) {
	uint byteSize = 32;

	__m256i hashed = SimdHelper::remainder_epu32(
			_mm256_mullo_epi32(input, HASH_FACTOR), bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = SimdHelper::divrem_epu32(&offset, hashed, byteSize);
	__m256i index2n = _mm256_add_epi32(index, index);

	// Use index to load from bitmap, the scale here is byte, thus load 32 bit integer use scale 4.
	__m256i byte = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);
	// Use offset to create pattern
	__m256i ptn = _mm256_sllv_epi32(ONE, offset);
	// -1 for selected key, zero for abandoned key
	__m256i selector = _mm256_srav_epi32(_mm256_and_si256(byte, ptn), offset);
	selector = _mm256_sign_epi32(selector, MAX);
	return _mm256_and_si256(selector, input);
}

/**
 * Return the location of the given key in cht payload, -1 if not found
 */
__m256i CStepSimd::lookup_cht(ulong* bitmap, uint bitmapSize, uint* chtpayload,
		uint chtsize, __m256i input) {
	__m256i hashed = SimdHelper::remainder_epu32(
			_mm256_mullo_epi32(input, HASH_FACTOR), bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = SimdHelper::divrem_epu32(&offset, hashed, BITMAP_UNIT);
	__m256i index2n = _mm256_add_epi32(index, index);
	__m256i index2n1 = _mm256_add_epi32(index2n, ONE);

	__m256i basePop = _mm256_i32gather_epi32((int* )bitmap, index2n1, 4);
	__m256i loadOffset = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);

	__m256i mask = _mm256_xor_si256(_mm256_srav_epi32(MAX, offset), MAX);
	__m256i partialPop = SimdHelper::popcnt_epi32(
			_mm256_and_si256(loadOffset, mask));

	__m256i location = _mm256_add_epi32(basePop, partialPop);

	__m256i result = ZERO;

	for (int i = 0; i < THRESHOLD; i++) {
		__m256i chtval = _mm256_i32gather_epi32((int* )chtpayload, location, 4);
		// a value of 0 means found
		__m256i compare = _mm256_xor_si256(input, chtval);
		// Test function is not available in AVX2, write my own
		__m256i locmask = SimdHelper::testz_epi32(compare);
		// Store location + 1 in result, 0 for not found
		__m256i locstore = _mm256_and_si256(locmask,
				_mm256_add_epi32(location, ONE));
		result = _mm256_or_si256(result, locstore);
		location = _mm256_add_epi32(location, ONE);
		// If location is greater than boundary, reduce it
		location = SimdHelper::remainder_epu32(location, chtsize);
	}
	// Location for found key, and -1 for not found
	return _mm256_sub_epi32(result, ONE);
}

CStepSimd::CStepSimd() {

}

CStepSimd::~CStepSimd() {
	if (NULL != probe)
		::free(probe);
	probe = NULL;
	if (alignedBitmap != NULL)
		::free(alignedBitmap);
	if (alignedChtload != NULL)
		::free(alignedChtload);
}

void CStepSimd::buildProbe(kvlist* inner) {
	this->probeSize = inner->size;
	this->probe = (uint*) ::aligned_alloc(32, sizeof(uint) * probeSize);
	for (uint i = 0; i < probeSize; i++) {
		probe[i] = inner->entries[i].key;
	}
}

void CStepSimd::init() {
	// Allocate aligned bitmap
	alignedBitmap = (ulong*) ::aligned_alloc(32,
			sizeof(ulong) * _lookup->bitmap_size);
	::memcpy(alignedBitmap, _lookup->bitmap,
			sizeof(ulong) * _lookup->bitmap_size);

	alignedChtload = (uint*) ::aligned_alloc(32,
			sizeof(uint) * _lookup->payload_size);
	::memcpy(alignedChtload, _lookup->keys,
			sizeof(uint) * _lookup->payload_size);
}

uint CStepSimd::filter(uint* gathered) {
	CheckBitmapTransform cbt(this);
	SimdHelper::transform(probe, probeSize, gathered, &cbt);
	// Do not filter
	return probeSize;
}

uint CStepSimd::lookup(uint* key, uint keylength, uint* result) {
	LookupChtTransform lct(this);
	SimdHelper::transform(key, keylength, result, &lct);
	return 0;
}

__m256i CheckBitmapTransform::transform(__m256i input) {
	return CStepSimd::check_bitmap(owner->alignedBitmap,
			owner->_lookup->bitmapSize(), input);
}

__m256i LookupChtTransform::transform(__m256i input) {
	return CStepSimd::lookup_cht(owner->alignedBitmap,
			owner->_lookup->bitmapSize(), owner->alignedChtload,
			owner->_lookup->payload_size, input);
}

