/*
 * SimdCHTJoin_test.cpp
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <stdlib.h>
#include "../src/join/simd/SimdCHTJoin.h"

class SimdCHTJoinTester: public SimdCHTJoin {
public:
	void set(ulong* bitmap, uint* chtload, uint* hashbkt) {
		alignedBitmap = bitmap;
		alignedChtload = chtload;
		alignedHashbkt = hashbkt;
	}

	void testLoadBitmap(__m256i input, __m256i* base, __m256i* byte) {
		load_bitmap(input, base,byte);
	}

	void testCount(__m256i input) {
		count(input);
	}

	__m256i testFilter(__m256i input) {
		return filter(input);
	}

	__m256i testCheckCht(__m256i location, __m256i key) {
		return check_cht(location, key);
	}

	void testCheckHash(__m256i input) {
		check_hash(input);
	}

	void testProcess(__m256i input) {
		process(input);
	}

	void testProcessDone() {
		processDone();
	}
};

TEST(SimdCHTJoin, LoadBitmap) {

	SimdCHTJoinTester *join = new SimdCHTJoinTester();
	ulong* bitmap = (ulong*) aligned_alloc(32, sizeof(ulong) * 40);
	for (int i = 0; i < 40; i++) {
		long top = 2 * i + 3;
		long bottom = 3 * i + 5;
		bitmap[i] = (top << 32) + bottom;
	}
	join->setBitmap(bitmap);

	__m256i input = _mm256_setr_epi32(3, 7, 6, 2, 11, 29, 8, 34);
	__m256i base;
	__m256i byte;

	join->testLoadBitmap(input, &base, &byte);

	uint* inputview = (uint*) &input;
	uint* baseview = (uint*) &base;
	uint* byteview = (uint*) &byte;

	for (uint i = 0; i < 8; i++) {
		ASSERT_EQ(inputview[i] * 2 + 3, baseview[i]);
		ASSERT_EQ(inputview[i] * 3 + 5, byteview[i]);
	}
}

TEST(SimdCHTJoin, Count) {
	FAIL()<< "Not implemented";
}

TEST(SimdCHTJoin, Filter) {
	FAIL()<< "Not implemented";
}

TEST(SimdCHTJoin, CheckCht) {
	FAIL()<< "Not implemented";
}

TEST(SimdCHTJoin, CheckHash) {
	FAIL()<< "Not implemented";
}

TEST(SimdCHTJoin, Process) {
	FAIL()<< "Not implemented";
}

TEST(SimdCHTJoin, ProcessDone) {
	FAIL()<< "Not implemented";
}

