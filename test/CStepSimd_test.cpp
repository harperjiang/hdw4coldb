/*
 * CStepSimd_test.cpp
 *
 *  Created on: Dec 21, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <stdlib.h>
#include "../src/join/cstep/CStepSimd.h"

TEST(CStepSimd, genBitmap) {
	CHT* cht = new CHT();
	kvlist* list = new kvlist();
	list->entries = new kv[20];
	for(int i = 0 ; i < 20 ; i++) {
		list->entries[i].key = 3*i+5;
	}
	list->size = 20;
	cht->build(list->entries,list->size);

	for(int i = 0 ; i < cht->bitmapSize();i++) {
		printf("%#lx\n",cht->bitmap[i]);
	}
}

TEST(CStepSimd, check_bitmap) {

	uint bitmapSize = 20;
	ulong data[bitmapSize] = {0x81000, 0x‭280044880‬, 0x‭744002040‬, 0x‭B02200122‬,
		0x‭1009100001‬, 0, 0, 0, 0, 7889, 140408603966328, 140408603966328, 0,
		0, 0, 0, 0, 0, 0, 0};
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
