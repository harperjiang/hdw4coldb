/*
 * Simd64VecBuffer.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#include "Simd64VecBuffer.h"

#include "../simd/SimdHelper.h"

__m256i Simd64VecBuffer::EMPTY = _mm256_setzero_si256();

namespace Simd64VecBufferConstants {

__m256i FLAG_SHIFT_64 = _mm256_setr_epi64x(3, 2, 1, 0);
__m256i LOOKUP_SIZE = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3,
		3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

// 0000
__m256i permute_64_0(__m256i a) {
	return a;
}
// 0001
__m256i permute_64_1(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x93);
}
// 0010
__m256i permute_64_2(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x72);
}
// 0011
__m256i permute_64_3(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x4e);
}
// 0100
__m256i permute_64_4(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0xe1);
}
// 0101
__m256i permute_64_5(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x2d);
}
// 0110
__m256i permute_64_6(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x39);
}
// 0111
__m256i permute_64_7(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x39);
}

// 1000
__m256i permute_64_8(__m256i a) {
	return a;
}
// 1001
__m256i permute_64_9(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x6c);
}
// 1010
__m256i permute_64_10(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x78);
}
// 1011
__m256i permute_64_11(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0x78);
}
// 1100
__m256i permute_64_12(__m256i a) {
	return a;
}
// 1101
__m256i permute_64_13(__m256i a) {
	return _mm256_permute4x64_epi64(a, 0xb4);
}
// 1110
__m256i permute_64_14(__m256i a) {
	return a;
}
// 1111
__m256i permute_64_15(__m256i a) {
	return a;
}

__m256i (*PERMU64[16])(
		__m256i) = {permute_64_0,permute_64_1,permute_64_2,permute_64_3,permute_64_4,permute_64_5,permute_64_6,permute_64_7,permute_64_8,permute_64_9,permute_64_10,permute_64_11,permute_64_12,permute_64_13,permute_64_14,permute_64_15};

__m256i blend_64_0(__m256i a, __m256i b) {
	return b;
}
__m256i blend_64_1(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xfc);
}
__m256i blend_64_2(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xf0);
}
__m256i blend_64_3(__m256i a, __m256i b) {
	return _mm256_blend_epi32(a, b, 0xc0);
}

__m256i (*BLEND64[4])(__m256i, __m256i)
		= {	blend_64_0,blend_64_1,blend_64_2,blend_64_3};

__m256i shift_64_0(__m256i a) {
	return a;
}

__m256i shift_64_1(__m256i a) {
	// 01101100
	return _mm256_permute4x64_epi64(a, 0x39);
}

__m256i shift_64_2(__m256i a) {
	// 10110001
	return _mm256_permute4x64_epi64(a, 0x4e);
}

__m256i shift_64_3(__m256i a) {
	// 11000110
	return _mm256_permute4x64_epi64(a, 0x93);
}

__m256i (*SHR64[4])(__m256i) = {shift_64_0, shift_64_3, shift_64_2,shift_64_1};
__m256i (*SHL64[4])(__m256i) = {shift_64_0,shift_64_1,shift_64_2,shift_64_3};

}

using namespace Simd64VecBufferConstants;

__m256i Simd64VecBuffer::align(__m256i input, int *inputSize) {
	__m256i flag = _mm256_add_epi64(_mm256_cmpeq_epi64(input, SimdHelper::ZERO),
			SimdHelper::ONE_64);
	__m256i sflag = _mm256_sllv_epi64(flag, FLAG_SHIFT_64);
	__m256i pflag = _mm256_permute4x64_epi64(sflag, 0x39);
	__m256i mflag = _mm256_add_epi64(sflag, pflag);
	__m256i m2flag = _mm256_permute4x64_epi64(mflag, 0x4e);
	mflag = _mm256_add_epi64(mflag, m2flag);
	// Each 64-bit in mflag contains flag
	int type = _mm256_extract_epi64(mflag, 0);
	// Little-Endian, lookup 8 bit
	__m256i sizev = _mm256_shuffle_epi8(LOOKUP_SIZE, mflag);
	int size = _mm256_extract_epi32(sizev, 0);
	*inputSize = size;
	if (size == 4)
		return input;

	return PERMU64[type](input);
}

__m256i Simd64VecBuffer::merge(__m256i a, __m256i b, int asize) {
	return BLEND64[asize](a, b);
}

__m256i Simd64VecBuffer::shl(__m256i a, int offset) {
	return SHL64[offset](a);
}

__m256i Simd64VecBuffer::shr(__m256i a, int offset) {
	return SHR64[offset](a);
}

Simd64VecBuffer::Simd64VecBuffer() {
	buffer = EMPTY;
	bufferSize = 0;
}

Simd64VecBuffer::~Simd64VecBuffer() {
}

__m256i Simd64VecBuffer::serve(__m256i input, int *outputSize) {
	int inputSize;
	__m256i aligned = align(input, &inputSize);

	int oldBufferSize = bufferSize;
	__m256i oldBuffer = buffer;

	int totalSize = inputSize + oldBufferSize;
	if (totalSize > 4) {
		bufferSize = totalSize - 4;
		buffer = shl(input, inputSize - bufferSize);

		*outputSize = 4;
		return merge(oldBuffer, shr(input, oldBufferSize), oldBufferSize);
	} else if (inputSize + oldBufferSize == 4) {
		bufferSize = 0;

		*outputSize = 4;
		return merge(oldBuffer, shr(input, oldBufferSize), oldBufferSize);
	} else {
		bufferSize = totalSize;
		buffer = merge(oldBuffer, shr(input, oldBufferSize), oldBufferSize);

		*outputSize = 0;
		return EMPTY;
	}
}

__m256i Simd64VecBuffer::purge(int *outputSize) {
	*outputSize = bufferSize;
	bufferSize = 0;
	return buffer;
}
