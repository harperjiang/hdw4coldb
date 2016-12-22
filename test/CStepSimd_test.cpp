/*
 * CStepSimd_test.cpp
 *
 *  Created on: Dec 21, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <stdlib.h>
#include "../src/join/cstep/CStepSimd.h"

TEST(CStepSimd, check_bitmap) {

	uint bitmapSize = 5;
	ulong data[bitmapSize] = {0x81000,
		0x280044880,
		0x744002040,
		0xb02200122,
		0x1009100001
	};

	uint* intview = (uint*)data;

	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			bitmapSize * sizeof(ulong));

	for (int i = 0; i < bitmapSize; i++) {
		alignedBitmap[i] = data[i];
	}

	__m256i input = _mm256_setr_epi32(3, 6, 2, 8, 11, 14, 25, 18);

	__m256i result = CStepSimd::check_bitmap(alignedBitmap, bitmapSize, input);

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

TEST(CStepSimd, lookup_cht) {
	uint bitmapSize = 5;
	ulong data[bitmapSize] = {0x81000,
		0x280044880,
		0x744002040,
		0xb02200122,
		0x1009100001
	};
	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			bitmapSize * sizeof(ulong));

	for (int i = 0; i < bitmapSize; i++) {
		alignedBitmap[i] = data[i];
	}
	uint chtsize = 20;
	uint chtpayload[chtsize] = {0x2c, 0x23, 0x17, 0x3b, 0xe, 0x32, 0x2f, 0x26, 0x1d, 0x1a, 0x3e, 0x11, 0x35, 0x8, 0x5, 0x29,0x20, 0x14, 0x38, 0xb};
	uint* alignedPayload = (uint*) aligned_alloc(32,
			chtsize * sizeof(uint));

	for (int i = 0; i < chtsize; i++) {
		alignedPayload[i] = chtpayload[i];
	}

	__m256i input = _mm256_setr_epi32(0, 0, 0, 8, 11, 14, 0, 0);

	__m256i result = CStepSimd::lookup_cht(alignedBitmap,bitmapSize, chtpayload,chtsize, input);
	uint* res = (uint*)&result;
	ASSERT_EQ(0xffffffff, res[0]);
	ASSERT_EQ(0xffffffff, res[1]);
	ASSERT_EQ(0xffffffff, res[2]);
	ASSERT_EQ(13, res[3]);
	ASSERT_EQ(11, res[4]);
	ASSERT_EQ(17, res[5]);
	ASSERT_EQ(0xffffffff, res[6]);
	ASSERT_EQ(0xffffffff, res[7]);

	free(alignedPayload);
	free(alignedBitmap);
}
