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

	ASSERT_EQ(5, m->getCounter());
	delete join;
}

TEST(SimdCHTJoin, Filter) {
	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5 * i + 6;
	}
	outer->entries = records;
	outer->size = 200;

	SimdCHTJoinTester *join = new SimdCHTJoinTester();

	join->testBuildLookup(outer);

	__m256i result = join->testFilter(
			_mm256_setr_epi32(1, 2, 3, 11, 5, 16, 7, 21));

	EXPECT_EQ(0, _mm256_extract_epi32(result,0));
	EXPECT_EQ(0, _mm256_extract_epi32(result,1));
	EXPECT_EQ(0, _mm256_extract_epi32(result,2));
	EXPECT_EQ(67, _mm256_extract_epi32(result,3));
	EXPECT_EQ(0, _mm256_extract_epi32(result,4));
	EXPECT_EQ(79, _mm256_extract_epi32(result,5));
	EXPECT_EQ(0, _mm256_extract_epi32(result,6));
	EXPECT_EQ(91, _mm256_extract_epi32(result,7));

	delete join;
}

TEST(SimdCHTJoin, CheckCht) {

	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5 * i + 6;
	}
	outer->entries = records;
	outer->size = 200;

	SimdCHTJoinTester *join = new SimdCHTJoinTester();

	join->testBuildLookup(outer);

	__m256i location = _mm256_setr_epi32(103, 172, 85, 67, 38, 79, 5, 91);
	__m256i key = _mm256_setr_epi32(26, 56, 106, 11, 86, 16, 506, 21);

	__m256i result = join->testCheckCht(location, key);

	EXPECT_EQ(0, _mm256_extract_epi32(result,0));
	EXPECT_EQ(0, _mm256_extract_epi32(result,1));
	EXPECT_EQ(0, _mm256_extract_epi32(result,2));
	EXPECT_EQ(0, _mm256_extract_epi32(result,3));
	EXPECT_EQ(0, _mm256_extract_epi32(result,4));
	EXPECT_EQ(0, _mm256_extract_epi32(result,5));
	EXPECT_EQ(506, _mm256_extract_epi32(result,6));
	EXPECT_EQ(0, _mm256_extract_epi32(result,7));

	ASSERT_EQ(7, join->getMatched()->getCounter());

	delete join;
}

TEST(SimdCHTJoin, CheckHash) {
	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5 * i + 6;
	}
	outer->entries = records;
	outer->size = 200;
	Hash* hash = new Hash();

	hash->build(records, 200);

//	uint* alignedbkts = (uint*)aligned_alloc(32, sizeof(uint)*hash->bucket_size);
//	memcpy(alignedbkts, hash->buckets, sizeof(uint)*hash->bucket_size);
//
	SimdCHTJoinTester* join = new SimdCHTJoinTester();
	join->set(NULL, NULL, hash->buckets);

	__m256i key = _mm256_setr_epi32(26, 56, 27, 11, 33, 16, 506, 21);

	join->testCheckHash(key);

	ASSERT_EQ(5, join->getMatched()->getCounter());

	delete join;
	delete hash;
}

TEST(SimdCHTJoin, Process) {
	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5 * i + 6;
	}
	outer->entries = records;
	outer->size = 200;

	SimdCHTJoinTester *join = new SimdCHTJoinTester();

	join->testBuildLookup(outer);

	__m256i key = _mm256_setr_epi32(26, 56, 106, 11, 86, 16, 506, 21);
	__m256i key2 = _mm256_setr_epi32(32, 1231, 41, 4535, 76, 334, 51, 5);

	join->testProcess(key);
	join->testProcess(key2);

	ASSERT_EQ(8, join->getMatched()->getCounter());

	delete join;
}

TEST(SimdCHTJoin, ProcessDone) {
	kv* records = new kv[200];
	kvlist* outer = new kvlist();

	for (int i = 0; i < 200; i++) {
		records[i].key = 5 * i + 6;
	}
	outer->entries = records;
	outer->size = 200;

	SimdCHTJoinTester *join = new SimdCHTJoinTester();

	join->testBuildLookup(outer);

	__m256i key = _mm256_setr_epi32(26, 56, 106, 11, 86, 16, 506, 21);
	__m256i key2 = _mm256_setr_epi32(32, 1231, 41, 4535, 76, 334, 51, 5);

	join->testProcess(key);
	join->testProcess(key2);
	join->testProcessDone();

	ASSERT_EQ(10, join->getMatched()->getCounter());

	delete join;
}

