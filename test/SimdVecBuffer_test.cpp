/*
 * VecBuffer_test.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include <immintrin.h>
#include "../src/vecbuffer/VecBuffer.h"
#include "../src/vecbuffer/SimdVecBuffer.h"

TEST(SimdVecBuffer, Serve) {
	SimdVecBuffer* sbuf = new SimdVecBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi32(3, 0, 0, 2, 7, 1, 4, 8);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi32(0, 2, 0, 99, 0, 0, 241, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(8, outputSize);
	ASSERT_EQ(3, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 1));
	ASSERT_EQ(7, _mm256_extract_epi32(output, 2));
	ASSERT_EQ(1, _mm256_extract_epi32(output, 3));
	ASSERT_EQ(4, _mm256_extract_epi32(output, 4));
	ASSERT_EQ(8, _mm256_extract_epi32(output, 5));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 6));
	ASSERT_EQ(99, _mm256_extract_epi32(output, 7));

	input = _mm256_setr_epi32(0, 0, 0, 2324, 0, 0, 0, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	delete sbuf;
}

TEST(SimdVecBuffer, Purge) {
	SimdVecBuffer* sbuf = new SimdVecBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi32(3, 0, 0, 2, 7, 1, 4, 8);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi32(0, 2, 0, 99, 0, 0, 241, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(8, outputSize);
	ASSERT_EQ(3, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 1));
	ASSERT_EQ(7, _mm256_extract_epi32(output, 2));
	ASSERT_EQ(1, _mm256_extract_epi32(output, 3));
	ASSERT_EQ(4, _mm256_extract_epi32(output, 4));
	ASSERT_EQ(8, _mm256_extract_epi32(output, 5));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 6));
	ASSERT_EQ(99, _mm256_extract_epi32(output, 7));

	input = _mm256_setr_epi32(0, 0, 0, 2324, 0, 0, 0, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	output = sbuf->purge(&outputSize);
	ASSERT_EQ(2, outputSize);
	ASSERT_EQ(241, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2324, _mm256_extract_epi32(output, 1));

	delete sbuf;
}
