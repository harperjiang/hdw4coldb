/*
 * SimdHelper.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: Cathy
 */

#include <x86intrin.h>
#include <memory.h>
#include "SimdHelper.h"
#include <stdio.h>

SimdHelper::SimdHelper() {
}

SimdHelper::~SimdHelper() {
}

__m256i SimdHelper::ZERO = _mm256_setzero_si256();
__m256i SimdHelper::ONE = _mm256_set1_epi32(1);
__m256i SimdHelper::ONE_64 = _mm256_set1_epi64x(1);
__m256i SimdHelper::TWO = _mm256_set1_epi32(2);
__m256i SimdHelper::FOUR = _mm256_set1_epi32(4);
__m256i SimdHelper::THIRTY_ONE = _mm256_set1_epi32(31);
__m256i SimdHelper::MAX = _mm256_set1_epi32(-1);

namespace SimdHelperConstants {
__m256i POPCNT_WWG_C1 = _mm256_set1_epi8(0x55);
__m256i POPCNT_WWG_C2 = _mm256_set1_epi8(0x33);
__m256i POPCNT_WWG_C3 = _mm256_set1_epi8(0x0F);
__m256i POPCNT_WWG_C4 = _mm256_set1_epi8(0x01);
__m256i POPCNT_MULA_C = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2,
		3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
}

using namespace SimdHelperConstants;

void SimdHelper::transform(uint* src, uint srclength, uint* dest,
		SimdTransform* trans, bool enableProfiling) {
	for (uint i = 0; i < srclength / 8; i++) {
		uint index = i * 8;
		__m256i loadkey = _mm256_load_si256((__m256i *) (src + index));
		__m256i* storeloc = (__m256i*)(dest+index);
		__m256i result = trans->transform(loadkey);
		_mm256_store_si256(storeloc, result);
	}
	if (srclength % 8) {
		uint psize = srclength % 8;
		uint index = (srclength / 8) * 8;
		uint* start = src + index;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		__m256i partialprocessed = trans->transform(loadpartial);
		store_epu32(dest, index, partialprocessed, psize);
	}
}

void SimdHelper::transform2(uint* srca, uint* srcb, uint srclength, uint* dest,
		SimdTransform* trans, bool enableProfiling) {
	for (uint i = 0; i < srclength / 8; i++) {
		uint index = i * 8;
		__m256i a = _mm256_load_si256((__m256i *) (srca + index));
		__m256i b = _mm256_load_si256((__m256i *) (srcb + index));
		__m256i processed = trans->transform2(a, b);
		__m256i* output = (__m256i*)(dest+index);
		_mm256_store_si256(output, processed);
	}
	if (srclength % 8) {
		uint psize = srclength % 8;
		uint index = (srclength / 8) * 8;
		uint* starta = srca + index;
		uint* startb = srcb + index;
		__m256i partiala = _mm256_setr_epi32(psize >= 1 ? starta[0] : 0,
				psize >= 2 ? starta[1] : 0, psize >= 3 ? starta[2] : 0,
				psize >= 4 ? starta[3] : 0, psize >= 5 ? starta[4] : 0,
				psize >= 6 ? starta[5] : 0, psize >= 7 ? starta[6] : 0, 0);

		__m256i partialb = _mm256_setr_epi32(psize >= 1 ? startb[0] : 0,
				psize >= 2 ? startb[1] : 0, psize >= 3 ? startb[2] : 0,
				psize >= 4 ? startb[3] : 0, psize >= 5 ? startb[4] : 0,
				psize >= 6 ? startb[5] : 0, psize >= 7 ? startb[6] : 0, 0);
		__m256i partialprocessed = trans->transform2(partiala, partialb);
		store_epu32(dest, index, partialprocessed, psize);
	}
}

void SimdHelper::transform3(uint* src, uint srclength, uint* dest1, uint* dest2,
		SimdTransform* trans, bool enableProfiling) {

	for (uint i = 0; i < srclength / 8; i++) {
		uint index = i * 8;
		__m256i loadkey = _mm256_load_si256((__m256i *) (src + index));

		__m256i *out = (__m256i *) (dest2 + index);
		__m256i processed = trans->transform3(loadkey, out);
		_mm256_store_si256((__m256i *) (dest1 + index), processed);
	}
	if (srclength % 8) {
		uint psize = srclength % 8;
		uint index = (srclength / 8) * 8;
		uint* start = src + index;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		__m256i out;
		__m256i partialprocessed = trans->transform3(loadpartial, &out);
		store_epu32(dest1, index, partialprocessed, psize);
		store_epu32(dest2, index, out, psize);
	}
}

void SimdHelper::print_epu32(__m256i a) {
	uint* data = (uint*) &a;
	printf("%x,%x,%x,%x,%x,%x,%x,%x\n", data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
}

void SimdHelper::print_m128_epu32(__m128i a) {
	uint* data = (uint*) &a;
	printf("%x,%x,%x,%x\n", data[0], data[1], data[2], data[3]);
}

__m256i SimdHelper::remainder_epu32(__m256i a, uint b) {
	uint* as = (uint*) &a;
	return _mm256_setr_epi32(as[0] % b, as[1] % b, as[2] % b, as[3] % b,
			as[4] % b, as[5] % b, as[6] % b, as[7] % b);
}

__m256i SimdHelper::divrem_epu32(__m256i* remainder, __m256i a, uint b) {
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

__m256i SimdHelper::popcnt_epi32_mula(__m256i input) {
	__m256i lower = _mm256_shuffle_epi8(POPCNT_MULA_C,
			_mm256_and_si256(input, POPCNT_WWG_C3));
	__m256i higher = _mm256_shuffle_epi8(POPCNT_MULA_C,
			_mm256_and_si256(_mm256_srli_epi32(input, 4), POPCNT_WWG_C3));
	__m256i byte = _mm256_add_epi32(lower, higher);
	__m256i result = _mm256_mullo_epi32(byte, POPCNT_WWG_C4);
	return _mm256_srli_epi32(result, 24);
}

__m256i SimdHelper::popcnt_epi32_wwg(__m256i input) {
	__m256i result = _mm256_sub_epi32(input,
			_mm256_and_si256(_mm256_srli_epi32(input, 1), POPCNT_WWG_C1));
	result = _mm256_add_epi32(
			_mm256_and_si256(_mm256_srli_epi32(result, 2), POPCNT_WWG_C2),
			_mm256_and_si256(result, POPCNT_WWG_C2));
	result = _mm256_add_epi32(
			_mm256_and_si256(_mm256_srli_epi32(result, 4), POPCNT_WWG_C3),
			_mm256_and_si256(result, POPCNT_WWG_C3));
	result = _mm256_mullo_epi32(result, POPCNT_WWG_C4);
	result = _mm256_srli_epi32(result, 24);
	return result;
}
/**
 * This implementation use the idea from https://arxiv.org/pdf/1611.07612v4.pdf
 */
__m256i SimdHelper::popcnt_epi32(__m256i input) {
	return popcnt_epi32_mula(input);
}

// set -1 for zero, 0 for non-zero
// Use test and setz assembly
__m256i SimdHelper::testz_epi32(__m256i input) {
	/*
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
	 */
	return _mm256_cmpeq_epi32(input, ZERO);
}

// set -1 for nz, 0 for zero
// Use test and setz assembly
__m256i SimdHelper::testnz_epi32(__m256i input) {
	return _mm256_xor_si256(testz_epi32(input), MAX);
	/*
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
	 return _mm256_sub_epi32(_mm256_setzero_si256(), result);*/
}

// Copy a portion of a SIMD result
void SimdHelper::store_epu32(uint* base, uint offset, __m256i input,
		uint length) {
	::memcpy(base + offset, &input, length * sizeof(int));
}

__m256i SimdTransform::transform(__m256i input) {
	return input;
}

__m256i SimdTransform::transform2(__m256i a, __m256i b) {
	return a;
}

__m256i SimdTransform::transform3(__m256i a, __m256i* out) {
	return a;
}
