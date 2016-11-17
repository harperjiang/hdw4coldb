/*
 * cht.c
 *
 *  Created on: Nov 16, 2016
 *      Author: Hao Jiang
 */

#include <stdlib.h>
#include <memory.h>
#include "cht.h"
#include "hash.h"
#include "util.h"

#define BITMAP_FACTOR 	8
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffff0000
#define BITMAP_MASK		0xffff

/**===========================================================================
 * Bitmap operations
 =============================================================================*/

/**
 * Test a bitmap location and set it to 1 if it is 0, return 1 if set
 */
u_char bitmap_testset(uint64_t* bitmap, uint32_t offset) {
	uint32_t bitmap_index = offset / BITMAP_UNIT;
	uint32_t bitmap_offset = offset % BITMAP_UNIT;

	uint32_t test = 0xffff & bitmap[bitmap_index] & 1 << (bitmap_offset);

	if (!test) {
		// Set
		bitmap[bitmap_index] |= bitmap_offset;
	}
	return !test;
}

/**
 * Set the high 32 bit in bitmap at offset to the given value
 */
void bitmap_setpopcnt(uint64_t* bitmap, uint32_t offset, uint32_t value) {
	bitmap[offset] &= BITMAP_EXTMASK;
	bitmap[offset] |= value << BITMAP_UNIT;
}

/**
 * Get popcount for the given hval
 */
uint32_t bitmap_popcnt(uint64_t* bitmap, uint32_t hval) {
	uint32_t bitmap_index = hval / BITMAP_UNIT;
	return (bitmap[bitmap_index] >> BITMAP_UNIT)
			+ popcnt(bitmap[bitmap_index] & BITMAP_MASK);

}

/**===========================================================================
 * CHT operations
 =============================================================================*/

/*
 * Build a CHT with given data, use two passing
 */
void cht_build(cht* cht, data* datas, uint32_t size) {
	uint32_t bitmap_size = BITMAP_FACTOR * size / BITMAP_UNIT;
	uint32_t bitsize = bitmap_size * BITMAP_UNIT;

	cht->bitmap_size = bitmap_size;
	cht->bitmap = (uint64_t*) malloc(sizeof(uint64_t) * bitmap_size);
	cht->payloads = (data*) malloc(sizeof(data) * size);
	cht->overflow = (hashtable*) malloc(sizeof(hashtable));
	hash_build(cht->overflow, size);

	// The first pass, fill in bitmap, use linear probing to resolve conflict
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = hash(datas[i].key) % bitsize;
		uint32_t counter = 0;
		while (counter < THRESHOLD
				&& !bitmap_testset(cht->bitmap, hval + counter)) {
			counter++;
		}
	}

	// update population in bitmap
	uint32_t sum = 0;
	for (uint32_t i = 0; i < bitmap_size; i++) {
		bitmap_setpopcnt(cht->bitmap, i, sum);
		sum += popcnt(cht->bitmap[i] >> BITMAP_UNIT);
	}

	// The second pass, allocate space and place items
	cht->payloads = (data*) malloc(sizeof(data) * sum);
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = hash(datas[i].key) % bitsize;
		uint32_t item_offset = bitmap_popcnt(cht->bitmap, hval);
		uint32_t counter = 0;

		while (counter < THRESHOLD
				&& cht->payloads[item_offset + counter] != NULL) {
			counter++;
		}
		if (counter == THRESHOLD) {
			// Goto overflow table
			hash_put(cht->overflow, datas[i].key, datas[i].payload);
		} else {
			cht->payloads[item_offset + counter] = datas[i];
		}
	}
}

/**
 * Looking for key in CHT
 */
uint8_t* cht_find(cht* cht, uint32_t key) {
	uint32_t hval = hash(key) % (cht->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(cht->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD && cht->payloads[offset + counter].key != key) {
		counter++;
	}
	if (counter == THRESHOLD) {
		return hash_get(cht->overflow, key);
	}
	return cht->payloads[offset + counter].payload;
}
