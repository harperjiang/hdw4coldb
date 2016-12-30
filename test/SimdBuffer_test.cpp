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
	// 10100110
	__m256i input = _mm256_setr_epi32(31331, 0, 22013, 0, 0, 124141, 551, 0);
	int size;
	__m256i result = SimdBuffer::align(input, &size);

	ASSERT_EQ(31331, _mm256_extract_epi32(result,0));
	ASSERT_EQ(22013, _mm256_extract_epi32(result,1));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,2));
	ASSERT_EQ(551, _mm256_extract_epi32(result,3));
	ASSERT_EQ(4, size);

	// 0000 1110
	input = _mm256_setr_epi32(0, 0, 0, 0, 5242152, 124141, 551, 0);
	result = SimdBuffer::align(input, &size);

	ASSERT_EQ(5242152, _mm256_extract_epi32(result,0));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,1));
	ASSERT_EQ(551, _mm256_extract_epi32(result,2));
	ASSERT_EQ(3, size);

	input = _mm256_setr_epi32(0, 0, 2719558, 0, 5242152, 124141, 551, 9895678);
	result = SimdBuffer::align(input, &size);

	ASSERT_EQ(2719558, _mm256_extract_epi32(result,0));
	ASSERT_EQ(5242152, _mm256_extract_epi32(result,1));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,2));
	ASSERT_EQ(551, _mm256_extract_epi32(result,3));
	ASSERT_EQ(9895678, _mm256_extract_epi32(result,4));
	ASSERT_EQ(5, size);
}

TEST(SimdBuffer, Shr) {
	__m256i input = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
	__m256i s1 = SimdBuffer::shr(input, 1);
	ASSERT_EQ(8, _mm256_extract_epi32(s1,0));
	ASSERT_EQ(1, _mm256_extract_epi32(s1,1));
	ASSERT_EQ(2, _mm256_extract_epi32(s1,2));
	ASSERT_EQ(3, _mm256_extract_epi32(s1,3));
	ASSERT_EQ(4, _mm256_extract_epi32(s1,4));
	ASSERT_EQ(5, _mm256_extract_epi32(s1,5));
	ASSERT_EQ(6, _mm256_extract_epi32(s1,6));
	ASSERT_EQ(7, _mm256_extract_epi32(s1,7));

	__m256i s2 = SimdBuffer::shr(input, 2);
	ASSERT_EQ(7, _mm256_extract_epi32(s2,0));
	ASSERT_EQ(8, _mm256_extract_epi32(s2,1));
	ASSERT_EQ(1, _mm256_extract_epi32(s2,2));
	ASSERT_EQ(2, _mm256_extract_epi32(s2,3));
	ASSERT_EQ(3, _mm256_extract_epi32(s2,4));
	ASSERT_EQ(4, _mm256_extract_epi32(s2,5));
	ASSERT_EQ(5, _mm256_extract_epi32(s2,6));
	ASSERT_EQ(6, _mm256_extract_epi32(s2,7));

	__m256i s3 = SimdBuffer::shr(input, 3);
	ASSERT_EQ(6, _mm256_extract_epi32(s3,0));
	ASSERT_EQ(7, _mm256_extract_epi32(s3,1));
	ASSERT_EQ(8, _mm256_extract_epi32(s3,2));
	ASSERT_EQ(1, _mm256_extract_epi32(s3,3));
	ASSERT_EQ(2, _mm256_extract_epi32(s3,4));
	ASSERT_EQ(3, _mm256_extract_epi32(s3,5));
	ASSERT_EQ(4, _mm256_extract_epi32(s3,6));
	ASSERT_EQ(5, _mm256_extract_epi32(s3,7));

	__m256i s4 = SimdBuffer::shr(input, 4);
	ASSERT_EQ(5, _mm256_extract_epi32(s4,0));
	ASSERT_EQ(6, _mm256_extract_epi32(s4,1));
	ASSERT_EQ(7, _mm256_extract_epi32(s4,2));
	ASSERT_EQ(8, _mm256_extract_epi32(s4,3));
	ASSERT_EQ(1, _mm256_extract_epi32(s4,4));
	ASSERT_EQ(2, _mm256_extract_epi32(s4,5));
	ASSERT_EQ(3, _mm256_extract_epi32(s4,6));
	ASSERT_EQ(4, _mm256_extract_epi32(s4,7));

	__m256i s5 = SimdBuffer::shr(input, 5);
	ASSERT_EQ(4, _mm256_extract_epi32(s5,0));
	ASSERT_EQ(5, _mm256_extract_epi32(s5,1));
	ASSERT_EQ(6, _mm256_extract_epi32(s5,2));
	ASSERT_EQ(7, _mm256_extract_epi32(s5,3));
	ASSERT_EQ(8, _mm256_extract_epi32(s5,4));
	ASSERT_EQ(1, _mm256_extract_epi32(s5,5));
	ASSERT_EQ(2, _mm256_extract_epi32(s5,6));
	ASSERT_EQ(3, _mm256_extract_epi32(s5,7));

	__m256i s6 = SimdBuffer::shr(input, 6);
	ASSERT_EQ(3, _mm256_extract_epi32(s6,0));
	ASSERT_EQ(4, _mm256_extract_epi32(s6,1));
	ASSERT_EQ(5, _mm256_extract_epi32(s6,2));
	ASSERT_EQ(6, _mm256_extract_epi32(s6,3));
	ASSERT_EQ(7, _mm256_extract_epi32(s6,4));
	ASSERT_EQ(8, _mm256_extract_epi32(s6,5));
	ASSERT_EQ(1, _mm256_extract_epi32(s6,6));
	ASSERT_EQ(2, _mm256_extract_epi32(s6,7));

	__m256i s7 = SimdBuffer::shr(input, 7);
	ASSERT_EQ(2, _mm256_extract_epi32(s7,0));
	ASSERT_EQ(3, _mm256_extract_epi32(s7,1));
	ASSERT_EQ(4, _mm256_extract_epi32(s7,2));
	ASSERT_EQ(5, _mm256_extract_epi32(s7,3));
	ASSERT_EQ(6, _mm256_extract_epi32(s7,4));
	ASSERT_EQ(7, _mm256_extract_epi32(s7,5));
	ASSERT_EQ(8, _mm256_extract_epi32(s7,6));
	ASSERT_EQ(1, _mm256_extract_epi32(s7,7));
}

TEST(SimdBuffer, Merge) {
	__m256i a = _mm256_setr_epi32(2, 4, 6, 8, 10, 12, 14, 16);
	__m256i b = _mm256_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15);

	__m256i merge = SimdBuffer::merge(a, b, 0);
	ASSERT_EQ(1, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(13, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(15, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 1);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(13, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 2);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 3);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 4);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 5);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 6);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(12, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,7));

	merge = SimdBuffer::merge(a, b, 7);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(12, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(14, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,7));
}
