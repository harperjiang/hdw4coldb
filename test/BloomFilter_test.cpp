/*
 * BloomFilter_test.cpp
 *
 *  Created on: Dec 27, 2016
 *      Author: Cathy
 */
#include <gtest/gtest.h>
#include "../src/lookup/BloomFilter.h"

TEST(BloomFilter, Build) {
	uint bfdata[3] = {37,221,4117};
	BloomFilter* bf = new BloomFilter(3,bfdata);
	kv* data = new kv[1000];
	for(int i = 0; i < 1000;i++) {
		data[i].key = i*3+5;
	}
	bf->build(data,1000);
	delete[] data;
	delete bf;
}

TEST(BloomFilter, Test) {
	uint bfdata[3] = {37,221,4117};
	BloomFilter* bf = new BloomFilter(3,bfdata);
	kv* data = new kv[1000];
	for(int i = 0; i < 1000;i++) {
		data[i].key = i*3+5;
	}
	bf->build(data,1000);
	for(int i = 0; i < 1000;i++) {
		ASSERT_TRUE(bf->test(3*i+5));
	}
	uint sum = 0;
	for(int i = 1000; i < 2000;i++) {
		sum+=bf->test(3*i+5);
	}
	ASSERT_TRUE(sum < 500);
	delete[] data;
	delete bf;
}
