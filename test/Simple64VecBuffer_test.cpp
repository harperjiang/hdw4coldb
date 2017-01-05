/*
 * VecBuffer_test.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include <immintrin.h>
#include "../src/vecbuffer/VecBuffer.h"
#include "../src/vecbuffer/Simple64VecBuffer.h"

TEST(Simple64VecBuffer, Serve) {
	Simple64VecBuffer* sbuf = new Simple64VecBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi64x(658, 31522, 0, 3141344);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi64x(4242442, 132131243, 1367650, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(4, outputSize);
	ASSERT_EQ(658, _mm256_extract_epi64(output, 0));
	ASSERT_EQ(31522, _mm256_extract_epi64(output, 1));
	ASSERT_EQ(3141344, _mm256_extract_epi64(output, 2));
	ASSERT_EQ(4242442, _mm256_extract_epi64(output, 3));

	input = _mm256_setr_epi64x(0, 0, 0, 29118);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	delete sbuf;
}

TEST(Simple64VecBuffer, Purge) {
	Simple64VecBuffer* sbuf = new Simple64VecBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi64x(658, 31522, 0, 3141344);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi64x(4242442, 132131243, 1367650, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(4, outputSize);
	ASSERT_EQ(658, _mm256_extract_epi64(output, 0));
	ASSERT_EQ(31522, _mm256_extract_epi64(output, 1));
	ASSERT_EQ(3141344, _mm256_extract_epi64(output, 2));
	ASSERT_EQ(4242442, _mm256_extract_epi64(output, 3));

	input = _mm256_setr_epi64x(0, 0, 0, 29118);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	output = sbuf->purge(&outputSize);
	ASSERT_EQ(3, outputSize);

	ASSERT_EQ(132131243, _mm256_extract_epi64(output, 0));
	ASSERT_EQ(1367650, _mm256_extract_epi64(output, 1));
	ASSERT_EQ(29118, _mm256_extract_epi64(output, 2));

	delete sbuf;
}
