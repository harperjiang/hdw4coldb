/*
 * CStepSimd.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStepSimd.h"
#include <x86intrin.h>
#include <stdlib.h>

#define BITMAP_UNIT 32

void print_epu32(__m256i a) {
	uint* data = (uint*) &a;
	printf("%x,%x,%x,%x,%x,%x,%x,%x\n", data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
}

__m256i remainder_epu32(__m256i a, uint b) {
	uint* as = (uint*) &a;
	return _mm256_setr_epi32(as[0] % b, as[1] % b, as[2] % b, as[3] % b,
			as[4] % b, as[5] % b, as[6] % b, as[7] % b);
}

__m256i divrem_epu32(__m256i* remainder, __m256i a, uint b) {
	__m256i quotient;
	uint* rem = (uint*)remainder;
	uint* quo = (uint*)&quotient;
	uint* as = (uint*)&a;
	for(uint i = 0; i < 8;i++) {
		asm volatile(
		"movl $0, %%edx\n\t"
		"movl %2,%%eax\n\t"
		"divl %3\n\t"
		"movl %%eax,%0\n\t"
		"movl %%edx,%1\n\t"
		:"=m"(quo[i]),"=m"(rem[i])
		:"m"(as[i]),"m"(b)
		:"edx","eax");
	}

	return quotient;
}

__m256i popcnt_epi32(__m256i input) {
	int* data = (int*) &input;
	return _mm256_setr_epi32(_popcnt32(data[0]), _popcnt32(data[1]),
			_popcnt32(data[2]), _popcnt32(data[3]), _popcnt32(data[4]),
			_popcnt32(data[5]), _popcnt32(data[6]), _popcnt32(data[7]));
}

// set -1 for zero, 0 for non-zero
// Use test and setz assembly
__m256i testz_epi32(__m256i input) {
	__m256i result;
	int* intinput = (int*) &input;
	int* intresult = (int*) &result;
	for (int i = 0; i < 8; i++) {
		asm volatile(
				"testl\t%1,%1\n\t"
				"setz\t%0\n\t"
				"andl\t$1,%0\n\t"
				: "=m"(intresult[i])
				: "r"(intinput[i])
				:"cc"
		);
	}
	return _mm256_sub_epi32(_mm256_setzero_si256(), result);
}

// set -1 for nz, 0 for zero
// Use test and setz assembly
__m256i testnz_epi32(__m256i input) {
	__m256i result;
	int* intinput = (int*) &input;
	int* intresult = (int*) &result;
	for (int i = 0; i < 8; i++) {
		asm volatile(
				"testl\t%1,%1\n\t"
				"setnz\t%0\n\t"
				"andl\t$1,%0\n\t"
				: "=m"(intresult[i])
				: "r"(intinput[i])
				:"cc"
		);
	}
	return _mm256_sub_epi32(_mm256_setzero_si256(), result);
}

void store_epi32(int* base, int offset, __m256i input) {
	::memcpy(base + offset, &input, 256);
}

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

	__m256i hashed = remainder_epu32(_mm256_mullo_epi32(input, HASH_FACTOR),
			bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = divrem_epu32(&offset, hashed, byteSize);
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
	__m256i hashed = remainder_epu32(_mm256_mullo_epi32(input, HASH_FACTOR),
			bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = divrem_epu32(&offset, hashed, BITMAP_UNIT);
	__m256i index2n = _mm256_add_epi32(index, index);
	__m256i index2n1 = _mm256_add_epi32(index2n, ONE);

	__m256i basePop = _mm256_i32gather_epi32((int* )bitmap, index2n1, 4);
	__m256i loadOffset = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);

	__m256i mask = _mm256_xor_si256(_mm256_srav_epi32(MAX, offset), MAX);
	__m256i partialPop = popcnt_epi32(_mm256_and_si256(loadOffset, mask));

	__m256i location = _mm256_add_epi32(basePop, partialPop);

	__m256i result = ZERO;

	for (int i = 0; i < THRESHOLD; i++) {
		__m256i chtval = _mm256_i32gather_epi32((int* )chtpayload, location, 4);
		// a value of 0 means found
		__m256i compare = _mm256_xor_si256(input, chtval);
		// Test function is not available in AVX2, write my own
		__m256i locmask = testz_epi32(compare);
		// Store location + 1 in result, 0 for not found
		__m256i locstore = _mm256_and_si256(locmask,
				_mm256_add_epi32(location, ONE));
		result = _mm256_or_si256(result, locstore);
		location = _mm256_add_epi32(location, ONE);
		// If location is greater than boundary, reduce it
		location = remainder_epu32(location, chtsize);
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
	for (uint i = 0; i < _lookup->payload_size; i++) {
		alignedChtload[i] = _lookup->payloads[i].key;
	}
}

uint CStepSimd::filter(uint* gathered) {
	for (uint i = 0; i < probeSize / 8; i++) {
		uint index = probe + i * 8;
		__m256i loadkey = _mm256_load_si256((__m256i *) index);
		__m256i check = check_bitmap(this->alignedBitmap,
				this->_lookup->bitmapSize(), loadkey);
		store_epi32(probe, i * 8, check);
	}
	if (probeSize % 8) {
		uint psize = probeSize % 8;
		uint* start = probe + (probeSize / 8) * 8;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		__m256i checkpartial = check_bitmap(this->alignedBitmap,
				this->_lookup->bitmapSize(), loadpartial);
	}

	// Do not filter
	return probeSize;
}

uint CStepSimd::lookup(uint* key, uint keylength, uint* result) {
	return 0;
}
