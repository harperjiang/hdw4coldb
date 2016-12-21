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

	uint bitmapSize = 20;
	ulong data[bitmapSize] = { 528384, 10737698944, 31205630016, 47280292130,
			68871520257, 0, 0, 0, 0, 7889, 140408603966328, 140408603966328, 0,
			0, 0, 0, 0, 0, 0, 0 };
	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			bitmapSize * sizeof(ulong));

	for (int i = 0; i < bitmapSize; i++) {
		alignedBitmap[0] = data[i];
	}

	__m256i input = _mm256_setr_epi32(3, 6, 2, 8, 11, 14, 25, 18);

	__m256i result = CStepSimd::check_bitmap(alignedBitmap, bitmapSize, input);

	uint* res = (uint*) &result;

	ASSERT_EQ(0, res[0]);
	ASSERT_EQ(0, res[1]);
	ASSERT_EQ(0, res[2]);
	ASSERT_EQ(1, res[3]);
	ASSERT_EQ(1, res[4]);
	ASSERT_EQ(1, res[5]);
	ASSERT_EQ(0, res[6]);
	ASSERT_EQ(0, res[7]);

	free(alignedBitmap);
}

TEST(CStepSimd, lookup_cht) {

}
