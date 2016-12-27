/*
 * BloomFilter.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: Cathy
 */

#include "BloomFilter.h"
#include "BloomFilterConstants.h"

BloomFilter::BloomFilter(uint size, uint* factors) {
	this->size = size;
	this->factors = factors;
	this->bitmaps = new ulong*[size];
	this->bitmapSize = 0;
}

BloomFilter::~BloomFilter() {
	for (uint i = 0; i < size; i++) {
		if (bitmaps[i] != NULL)
			delete[] bitmaps[i];
	}
	delete[] bitmaps;
}

void BloomFilter::build(kv* datas, uint data_size) {
	uint bitnum = data_size * BF_BITMAP_FACTOR;
	bitmapSize = bitnum / BF_BITMAP_UNIT + ((bitnum % BF_BITMAP_UNIT) ? 1 : 0);

	for (uint i = 0; i < size; i++) {
		bitmaps[i] = new ulong[bitmapSize];
	}

	for (uint i = 0; i < data_size; i++) {
		for (uint j = 0; j < size; j++) {
			uint hashed = factors[j] * datas[i].key;
			bitmaps[j][hashed / BF_BITMAP_UNIT] &= (1
					<< (hashed % BF_BITMAP_UNIT));
		}
	}
}

bool BloomFilter::test(uint key) {
	for (uint i = 0; i < size; i++) {
		uint hashed = factors[i] * key;
		if (!bitmaps[i][hashed / BF_BITMAP_UNIT]
				& (1 << hashed % BF_BITMAP_UNIT))
			return false;
	}
	return true;
}

