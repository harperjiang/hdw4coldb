/*
 * SimdCHTJoin_test.cpp
 *
 *  Created on: Dec 21, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <stdlib.h>
#include "../src/join/simd/SimdCHTJoin.h"
#include "../src/lookup/Hash.h"

TEST(SimdCHTJoin, check_bitmap) {

	uint bitmapSize = 5;
	ulong data[bitmapSize] = { 0x81000, 0x280044880, 0x744002040, 0xb02200122,
			0x1009100001 };

	uint* intview = (uint*) data;

	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			bitmapSize * sizeof(ulong));

	memcpy(alignedBitmap, data, sizeof(ulong) * bitmapSize);

	__m256i input = _mm256_setr_epi32(3, 6, 2, 8, 11, 14, 25, 18);

	__m256i result = SimdCHTJoin::check_bitmap(alignedBitmap, bitmapSize,
			input);

	uint* res = (uint*) &result;

	ASSERT_EQ(0, res[0]);
	ASSERT_EQ(0, res[1]);
	ASSERT_EQ(0, res[2]);
	ASSERT_EQ(8, res[3]);
	ASSERT_EQ(11, res[4]);
	ASSERT_EQ(14, res[5]);
	ASSERT_EQ(0, res[6]);
	ASSERT_EQ(0, res[7]);

	free(alignedBitmap);
}

TEST(SimdCHTJoin, lookup_cht) {
	uint bitmapSize = 5;
	ulong data[bitmapSize] = { 0x81000, 0x280044880, 0x744002040, 0xb02200122,
			0x1009100001 };
	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			bitmapSize * sizeof(ulong));

	memcpy(alignedBitmap, data, bitmapSize * sizeof(ulong));

	uint chtsize = 20;
	uint chtpayload[chtsize] =
			{ 0x2c, 0x23, 0x17, 0x3b, 0xe, 0x32, 0x2f, 0x26, 0x1d, 0x1a, 0x3e,
					0x11, 0x35, 0x8, 0x5, 0x29, 0x20, 0x14, 0x38, 0xb };
	uint* alignedPayload = (uint*) aligned_alloc(32, chtsize * sizeof(uint));

	memcpy(alignedPayload, chtpayload, sizeof(uint) * chtsize);

	__m256i input = _mm256_setr_epi32(2, 1, 0, 8, 11, 14, 0, 0);
	__m256i remain;
	__m256i result = SimdCHTJoin::lookup_cht(alignedBitmap, bitmapSize,
			chtpayload, chtsize, input, &remain);
	uint* res = (uint*) &result;
	ASSERT_EQ(0xffffffff, res[0]);
	ASSERT_EQ(0xffffffff, res[1]);
	ASSERT_EQ(0xffffffff, res[2]);
	ASSERT_EQ(13, res[3]);
	ASSERT_EQ(19, res[4]);
	ASSERT_EQ(4, res[5]);
	ASSERT_EQ(0xffffffff, res[6]);
	ASSERT_EQ(0xffffffff, res[7]);

	uint* rem = (uint*) &remain;
	ASSERT_EQ(2, rem[0]);
	ASSERT_EQ(1, rem[1]);
	ASSERT_EQ(0, rem[2]);
	ASSERT_EQ(0, rem[3]);
	ASSERT_EQ(0, rem[4]);
	ASSERT_EQ(0, rem[5]);
	ASSERT_EQ(0, rem[6]);
	ASSERT_EQ(0, rem[7]);

	free(alignedPayload);
	free(alignedBitmap);
}

TEST(SimdCHTJoin, lookup_hash) {

	uint datasize = 33;
	uint data[datasize] = { 8, 0, 44, 35, 0, 5, 53, 62, 0, 23, 14, 0, 0, 32, 41,
			0, 0, 50, 59, 0, 20, 11, 0, 0, 38, 29, 47, 0, 56, 0, 0, 26, 17 };
	uint* alignedbuckets = (uint*) aligned_alloc(32, sizeof(uint) * datasize);
	memcpy(alignedbuckets, data, sizeof(uint) * datasize);

	__m256i input = _mm256_setr_epi32(1, 0, 21, 8, 11, 14, 0, 0);

	__m256i result = SimdCHTJoin::lookup_hash(alignedbuckets, datasize, input);

	uint* res = (uint*) &result;
	ASSERT_EQ(0xffffffff, res[0]);
	ASSERT_EQ(0xffffffff, res[1]);
	ASSERT_EQ(0xffffffff, res[2]);
	ASSERT_EQ(0, res[3]);
	ASSERT_EQ(21, res[4]);
	ASSERT_EQ(10, res[5]);
	ASSERT_EQ(0xffffffff, res[6]);
	ASSERT_EQ(0xffffffff, res[7]);

	result = SimdCHTJoin::lookup_hash(alignedbuckets, datasize,
			SimdHelper::ZERO);

	res = (uint*) &result;
	ASSERT_EQ(0xffffffff, res[0]);
	ASSERT_EQ(0xffffffff, res[1]);
	ASSERT_EQ(0xffffffff, res[2]);
	ASSERT_EQ(0xffffffff, res[3]);
	ASSERT_EQ(0xffffffff, res[4]);
	ASSERT_EQ(0xffffffff, res[5]);
	ASSERT_EQ(0xffffffff, res[6]);
	ASSERT_EQ(0xffffffff, res[7]);

	free(alignedbuckets);
}
