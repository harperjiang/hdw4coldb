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
//	__m256i remainder = remainder_epu32(input,8);
//
//	uint* res = (uint*)&remainder;
//
//	ASSERT_EQ(1, res[0]);
//	ASSERT_EQ(2, res[1]);
//	ASSERT_EQ(3, res[2]);
//	ASSERT_EQ(4, res[3]);
//	ASSERT_EQ(5, res[4]);
//	ASSERT_EQ(6, res[5]);
//	ASSERT_EQ(7, res[6]);
//	ASSERT_EQ(0, res[7]);
}

TEST(SIMD, Divrem) {

}

TEST(SIMD, Popcnt) {

}

TEST(SIMD, Testz) {

}
