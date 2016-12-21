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
	CHT* cht = new CHT();
	kvlist* list = new kvlist();
	list->entries = new kv[20];
	list->size = 20;
	for (uint i = 0; i < 20; i++) {
		list->entries[i].key = 3 * i + 5;
	}

	cht->build(list->entries, list->size);

	ulong* alignedBitmap = (ulong*) aligned_alloc(32,
			cht->bitmapSize() * sizeof(ulong));
	memcpy(alignedBitmap, cht->bitmap, sizeof(ulong) * cht->bitmapSize());

	__m256i input = _mm256_setr_epi32(3, 6, 2, 8, 11, 14, 25, 18);

	__m256i result = CStepSimd::check_bitmap(alignedBitmap, cht->bitmapSize(),
			input);

	uint* res = (uint*) &result;

	ASSERT_EQ(0, res[0]);
	ASSERT_EQ(0, res[1]);
	ASSERT_EQ(0, res[2]);
	ASSERT_EQ(1, res[3]);
	ASSERT_EQ(1, res[4]);
	ASSERT_EQ(1, res[5]);
	ASSERT_EQ(0, res[6]);
	ASSERT_EQ(0, res[7]);

	delete[] list->entries;
	delete cht;
	delete list;
}

TEST(CStepSimd, lookup_cht) {

}
