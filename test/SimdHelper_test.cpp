/*
 * simd_test.cpp
 *
 *  Created on: Dec 20, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <immintrin.h>
#include <stdlib.h>

#include "../src/simd/SimdHelper.h"

TEST(SimdHelper, Remainder) {
	__m256i input = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
	__m256i remainder = SimdHelper::remainder_epu32(input, 8);

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

TEST(SimdHelper, Divrem) {
	__m256i input = _mm256_setr_epi32(47, 32, 65, 142, 251, 356, 27, 18);
	__m256i remainder;

	__m256i quotient = SimdHelper::divrem_epu32(&remainder, input, 8);
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

TEST(SimdHelper, Popcnt) {
	__m256i input = _mm256_setr_epi32(0x3242, 0x3, 0x242, 0x21, 0x19, 0x221,
			0xffffffff, 0xf2df);
	__m256i result = SimdHelper::popcnt_epi32(input);

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

TEST(SimdHelper, Testz) {
	__m256i input = _mm256_setr_epi32(32, 12, -1, 0, 5, 2, 0, -431);

	__m256i result = SimdHelper::testz_epi32(input);

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

TEST(SimdHelper, TestNz) {
	__m256i input = _mm256_setr_epi32(32, 12, -1, 0, 5, 2, 0, -431);

	__m256i result = SimdHelper::testnz_epi32(input);

	uint* res = (uint*) &result;

	ASSERT_EQ(0xffffffff, res[0]);
	ASSERT_EQ(0xffffffff, res[1]);
	ASSERT_EQ(0xffffffff, res[2]);
	ASSERT_EQ(0, res[3]);
	ASSERT_EQ(0xffffffff, res[4]);
	ASSERT_EQ(0xffffffff, res[5]);
	ASSERT_EQ(0, res[6]);
	ASSERT_EQ(0xffffffff, res[7]);
}

class TestTransform: public SimdTransform {

	virtual __m256i transform(__m256i input) {
		return _mm256_sub_epi32(SimdHelper::ZERO, input);
	}

	virtual __m256i transform2(__m256i a, __m256i b) {
		return _mm256_add_epi32(a, b);
	}
};

TEST(SimdHelper, Transform) {
	int* src = (int*) aligned_alloc(32, 20 * sizeof(int));
	for (int i = 0; i < 20; i++) {
		src[i] = 3 * i + 25;
	}
	int* dest = new int[20];
	TestTransform bt;
	SimdHelper::transform((uint*) src, 20, (uint*) dest, &bt);

	for (uint i = 0; i < 20; i++) {
		ASSERT_FALSE(src[i] + dest[i]);
	}

	free(src);
	delete[] dest;
}

TEST(SimdHelper, Transform2) {
	int* srca = (int*) aligned_alloc(32, 20 * sizeof(int));
	int* srcb = (int*) aligned_alloc(32, 20 * sizeof(int));
	for (int i = 0; i < 20; i++) {
		srca[i] = 6 * i + 7;
		srcb[i] = 25 * i + 235;
	}
	int* dest = new int[20];
	TestTransform bt;
	SimdHelper::transform2((uint*) srca, (uint*) srcb, 20, (uint*) dest, &bt);

	for (uint i = 0; i < 20; i++) {
		ASSERT_EQ(dest[i], srca[i] + srcb[i]);
	}

	free(srca);
	free(srcb);
	delete[] dest;
}

