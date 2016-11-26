/*
 * CHT.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */

#include <cstring>
#include "CHT.h"
#include "util.h"

#define THRESHOLD 5
#define BITMAP_FACTOR 	4
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

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

// The first pass, fill in bitmap,
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = hash(entries[i].key) % bitsize;
		bitmap_testset(this->bitmap, hval);
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
		uint32_t hval = hash(entries[i].key) % bitsize;
		uint32_t item_offset = bitmap_popcnt(this->bitmap, hval);
		uint32_t counter = 0;

		while (counter < THRESHOLD
				&& this->payloads[item_offset + counter].key != 0) {
			counter++;
		}
		kv* entry = entries + i;
		if (counter == THRESHOLD) {
			// Goto overflow table
			overflow->put(entry->key, entry->payload);
		} else {
			this->payloads[item_offset + counter].key = entry->key;
			::memcpy(this->payloads[item_offset + counter].payload,
					entry->payload,
					PAYLOAD_SIZE * sizeof(uint8_t));
		}
	}
	overflow->organize(overflow->size() * RATIO);
}

bool CHT::has(uint32_t key) {
	uint32_t hval = hash(key) % (bitmap_size * BITMAP_UNIT);
	return bitmap_test(bitmap, hval);
}

uint32_t CHT::payloadSize() {
	return this->payload_size;
}

uint32_t CHT::bitmapSize() {
	return this->bitmap_size;
}

kv* CHT::findUnique(uint32_t key) {
	if (!has(key))
		return NULL;
	uint32_t hval = hash(key) % (this->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(this->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD && this->payloads[offset + counter].key != key) {
		counter++;
	}
	if (counter == THRESHOLD) {
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
	uint32_t hval = hash(key) % (this->bitmap_size * BITMAP_UNIT);
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

Hash* CHT::getOverflow() {
	return overflow;
}
