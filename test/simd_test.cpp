/*
 * simd_test.cpp
 *
 *  Created on: Dec 20, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <immintrin.h>

extern __m256i remainder_epu32(__m256i a, uint b);
extern __m256i divrem_epu32(__m256i* remainder, __m256i a, uint b);
extern __m256i popcnt_epi32(__m256i input);
extern __m256i testz_epi32(__m256i input);

TEST(SIMD, Remainder) {
	__m256i input = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
	__m256i remainder = remainder_epu32(input, 8);

	uint* res = (uint*) &remainder;

	ASSERT_EQ(1, res[0]);
	ASSERT_EQ(2, res[1]);
	ASSERT_EQ(3, res[2]);
	ASSERT_EQ(4, res[3]);
	ASSERT_EQ(5, res[4]);
	ASSERT_EQ(6, res[5]);
	ASSERT_EQ(7, res[6]);
	ASSERT_EQ(0, res[7]);
}

TEST(SIMD, Divrem) {
	__m256i input = _mm256_setr_epi32(47, 32, 65, 142, 251, 356, 27, 18);
	__m256i remainder;

	__m256i quotient = divrem_epu32(&remainder, input, 8);
	uint* res = (uint*) &remainder;

	ASSERT_EQ(7, res[0]);
	ASSERT_EQ(0, res[1]);
	ASSERT_EQ(1, res[2]);
	ASSERT_EQ(6, res[3]);
	ASSERT_EQ(3, res[4]);
	ASSERT_EQ(4, res[5]);
	ASSERT_EQ(3, res[6]);
	ASSERT_EQ(2, res[7]);

	uint* quo = (uint*) &quotient;

	ASSERT_EQ(5, quo[0]);
	ASSERT_EQ(4, quo[1]);
	ASSERT_EQ(8, quo[2]);
	ASSERT_EQ(17, quo[3]);
	ASSERT_EQ(31, quo[4]);
	ASSERT_EQ(44, quo[5]);
	ASSERT_EQ(3, quo[6]);
	ASSERT_EQ(2, quo[7]);
}

TEST(SIMD, Popcnt) {
	__m256i input = _mm256_setr_epi32(0x3242, 0x3, 0x242, 0x21, 0x19, 0x221,
			0xffffffff, 0xf2df);
	__m256i result = popcnt_epi32(input);

	uint* res = (uint*) &result;

	ASSERT_EQ(5, res[0]);
	ASSERT_EQ(2, res[1]);
	ASSERT_EQ(3, res[2]);
	ASSERT_EQ(2, res[3]);
	ASSERT_EQ(3, res[4]);
	ASSERT_EQ(3, res[5]);
	ASSERT_EQ(32, res[6]);
	ASSERT_EQ(12, res[7]);
}

TEST(SIMD, Testz) {
	__m256i input = _mm256_setr_epi32(32, 12, -1, 0, 5, 2, 0, -431);

	__m256i result = testz_epi32(input);

	uint* res = (uint*) &result;

	ASSERT_EQ(0, res[0]);
	ASSERT_EQ(0, res[1]);
	ASSERT_EQ(0, res[2]);
	ASSERT_EQ(0xffffffff, res[3]);
	ASSERT_EQ(0, res[4]);
	ASSERT_EQ(0, res[5]);
	ASSERT_EQ(0xffffffff, res[6]);
	ASSERT_EQ(0, res[7]);
}
