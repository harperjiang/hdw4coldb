/*
 * SimdBuffer_test.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include <immintrin.h>
#include "../src/simd/SimdBuffer.h"

TEST(SimdBuffer, Align) {
	// 10010011
	__m256i input = _mm256_setr_epi32(31331, 0, 22013, 0, 0, 124141, 551, 0);
	int size;
	__m256i result = SimdBuffer::align(input, &size);

	ASSERT_EQ(31331, _mm256_extract_epi32(result,0));
	ASSERT_EQ(22013, _mm256_extract_epi32(result,1));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,2));
	ASSERT_EQ(551, _mm256_extract_epi32(result,3));
	ASSERT_EQ(4, size);
}

TEST(SimdBuffer, Shr) {

}
