/*
 * CHT.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */

#include <cstring>
#include <iostream>
#include <cstdio>
#include "CHT.h"
#include "util.h"

#define THRESHOLD 5
#define BITMAP_FACTOR 	8
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

using namespace std;

/**===========================================================================
 * Bitmap operations
 =============================================================================*/

/**
 * Test a bitmap location and set it to 1 if it is 0, return 1 if set
 */
bool bitmap_testset(uint64_t* bitmap, uint32_t offset) {
	uint32_t bitmap_index = offset / BITMAP_UNIT;
	uint32_t bitmap_offset = offset % BITMAP_UNIT;
	uint32_t mask = 1 << bitmap_offset;
	uint32_t test = BITMAP_MASK & bitmap[bitmap_index] & mask;

	if (!test) {
		// Set
		bitmap[bitmap_index] |= mask;
	}
	return !test;
}

bool bitmap_test(uint64_t* bitmap, uint32_t offset) {
	uint32_t bitmap_index = offset / BITMAP_UNIT;
	uint32_t bitmap_offset = offset % BITMAP_UNIT;
	uint32_t mask = 1 << bitmap_offset;
	return BITMAP_MASK & bitmap[bitmap_index] & mask;
}

/**
 * Set the high 32 bit in bitmap at offset to the given value
 */
void bitmap_setpopcnt(uint64_t* bitmap, uint32_t offset, uint32_t value) {
	bitmap[offset] |= ((uint64_t) value) << BITMAP_UNIT;
}

/**
 * Get popcount for the given hval, this equals to the value at upper half
 * and number of 1 in lower half up to hval
 */
uint32_t bitmap_popcnt(uint64_t* bitmap, uint32_t hval) {
	uint32_t bitmap_index = hval / BITMAP_UNIT;
	uint32_t bitmap_upto = hval % BITMAP_UNIT;

	uint32_t pop_before = (bitmap[bitmap_index] & BITMAP_EXTMASK) >> BITMAP_UNIT;
	if (bitmap_upto == 0)
		return pop_before;

	uint64_t fullmask = 0xffffffffffffffff;
	uint32_t pop_upto = bitmap[bitmap_index] & ~(fullmask << bitmap_upto);

	return pop_before + popcount(pop_upto);
}

CHT::CHT() :
		Lookup("CHT") {
	bitmap = NULL;
	bitmap_size = 0;
	payloads = NULL;
	payload_size = 0;
	overflow = NULL;
}

CHT::~CHT() {
	if (NULL != bitmap)
		delete[] bitmap;
	if (NULL != payloads)
		delete[] payloads;
	if (NULL != overflow)
		delete overflow;
}

void CHT::build(kv* entries, uint32_t size) {
	uint32_t bitnumber = BITMAP_FACTOR * size;
	uint32_t bitmap_size = bitnumber / BITMAP_UNIT;
	bitmap_size += (bitnumber % BITMAP_UNIT) ? 1 : 0;
	uint32_t bitsize = bitmap_size * BITMAP_UNIT;

	this->bitmap_size = bitmap_size;
	this->bitmap = new uint64_t[bitmap_size]();

	uint32_t initsize =
			(size / OVERFLOW_INIT) > MIN_SIZE ?
					(size / OVERFLOW_INIT) : MIN_SIZE;
	this->overflow = new Hash(initsize);

	// The first pass, fill in bitmap, do linear probing on collision
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = mut_hash(entries[i].key) % bitsize;
		uint32_t counter = 0;
		while (counter < THRESHOLD && hval + counter < bitsize
				&& !bitmap_testset(this->bitmap, hval + counter)) {
			counter++;
		}
		if (counter == THRESHOLD || hval + counter == bitsize) {
			this->overflow->put(entries[i].key, entries[i].payload);
		}
	}
	// update population in bitmap
	uint32_t sum = 0;
	for (uint32_t i = 0; i < bitmap_size; i++) {
		bitmap_setpopcnt(this->bitmap, i, sum);
		sum += popcount(this->bitmap[i] & BITMAP_MASK);
	}
	this->payload_size = sum;

	// The second pass, allocate space and place items
	this->payloads = new kv[sum];
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = mut_hash(entries[i].key) % bitsize;
		uint32_t item_offset = bitmap_popcnt(this->bitmap, hval);

		for (uint32_t counter = 0; counter < THRESHOLD; counter++) {
			kv* place = this->payloads + item_offset + counter;
			if (place->key == 0) {
				::memcpy(place, entries + i, sizeof(kv));
				break;
			}
		}
	}
	overflow->organize(overflow->size() * RATIO);
}

bool CHT::has(uint32_t key) {
	uint32_t hval = mut_hash(key) % (bitmap_size * BITMAP_UNIT);
	if (!bitmap_test(bitmap, hval)) {
		return false;
	}
	return NULL != this->findUnique(key);
}

uint32_t CHT::payloadSize() {
	return this->payload_size;
}

uint32_t CHT::bitmapSize() {
	return this->bitmap_size;
}

kv* CHT::findUnique(uint32_t key) {
	uint32_t hval = mut_hash(key) % (bitmap_size * BITMAP_UNIT);
	if (!bitmap_test(bitmap, hval)) {
		return NULL;
	}
	uint32_t offset = bitmap_popcnt(this->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD && offset + counter < payload_size
			&& this->payloads[offset + counter].key != key) {
		counter++;
	}
	if (counter == THRESHOLD || offset + counter >= payload_size) {
		return this->overflow->get(key);
	}
	return this->payloads + offset + counter;
}

uint8_t* CHT::access(uint32_t key) {
	kv* entry = this->findUnique(key);
	return (NULL != entry) ? entry->payload : NULL;
}

void CHT::scan(uint32_t key, ScanContext* context) {
	if (!has(key))
		return;
	uint32_t hval = mut_hash(key) % (this->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(this->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD) {
		if (this->payloads[offset + counter].key == key) {
			kv entry = this->payloads[offset + counter];
			context->execute(entry.key, entry.payload);
		}
		counter++;
	}
	this->overflow->scan(key, context);
}

Hash * CHT::getOverflow() {
	return overflow;
}
