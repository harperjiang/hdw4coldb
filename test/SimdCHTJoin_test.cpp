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
	void testBuildLookup(kvlist* outer) {
		buildLookup(outer);
	}

	void set(ulong* bitmap, uint* chtload, uint* hashbkt) {
		alignedBitmap = bitmap;
		alignedChtload = chtload;
		alignedHashbkt = hashbkt;
	}

	void set2(__m256i bitsize, __m256i bktsize) {
		this->bitsize = bitsize;
		this->bktsize = bktsize;
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

	Matched* getMatched() {
		return _matched;
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
	join->set(bitmap, NULL, NULL);

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
	SimdCHTJoinTester* join = new SimdCHTJoinTester();

	join->testCount(_mm256_setr_epi32(0, 4, 1, 5, 0, 0, 2, 7));

	Matched* m = join->getMatched();

	ASSERT_EQ(6, m->getCounter());
	delete join;
}

TEST(SimdCHTJoin, Filter) {
	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5i * 6;
	}
	outer->entries = records;
	outer->size = 200;

	SimdCHTJoinTester *join = new SimdCHTJoinTester();

	join->testBuildLookup(outer);

	__m256i result = join->testFilter(
			_mm256_setr_epi32(1, 2, 3, 11, 5, 16, 7, 21));

	ASSERT_EQ(0, _mm256_extract_epi32(result,0));
	ASSERT_EQ(0, _mm256_extract_epi32(result,1));
	ASSERT_EQ(0, _mm256_extract_epi32(result,2));
	ASSERT_EQ(0, _mm256_extract_epi32(result,3));
	ASSERT_EQ(0, _mm256_extract_epi32(result,4));
	ASSERT_EQ(0, _mm256_extract_epi32(result,5));
	ASSERT_EQ(0, _mm256_extract_epi32(result,6));
	ASSERT_EQ(0, _mm256_extract_epi32(result,7));

	delete join;
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

