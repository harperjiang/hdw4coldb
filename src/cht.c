/*
 * cht.c
 *
 *  Created on: Nov 16, 2016
 *      Author: Hao Jiang
 */

#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include "cht.h"
#include "hash.h"
#include "util.h"

#define BITMAP_FACTOR 	8
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

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

/**===========================================================================
 * CHT operations
 =============================================================================*/

/*
 * Build a CHT with given data, use two passing
 */
void cht_build(cht* cht, cht_entry* datas, uint32_t size) {
	uint32_t bitnumber = BITMAP_FACTOR * size;
	uint32_t bitmap_size = bitnumber / BITMAP_UNIT;
	bitmap_size += (bitnumber % BITMAP_UNIT) ? 1 : 0;
	uint32_t bitsize = bitmap_size * BITMAP_UNIT;

	cht->bitmap_size = bitmap_size;
	cht->bitmap = (uint64_t*) calloc(bitmap_size, sizeof(uint64_t));

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
		sum += popcount(cht->bitmap[i] & BITMAP_MASK);
	}
	cht->payload_size = sum;
	// The second pass, allocate space and place items
	cht->payloads = (cht_entry*) calloc(sum, sizeof(cht_entry));
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = hash(datas[i].key) % bitsize;
		uint32_t item_offset = bitmap_popcnt(cht->bitmap, hval);
		uint32_t counter = 0;

		while (counter < THRESHOLD
				&& cht->payloads[item_offset + counter].key != 0) {
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
cht_entry* cht_find_uniq(cht* cht, uint32_t key) {
	uint32_t hval = hash(key) % (cht->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(cht->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD && cht->payloads[offset + counter].key != key) {
		counter++;
	}
	if (counter == THRESHOLD) {
		return (cht_entry*) hash_get(cht->overflow, key);
	}
	return cht->payloads + offset + counter;
}

void cht_scan(cht* cht, uint32_t key,
		void (*scanfunc)(uint32_t key, uint8_t* payload)) {
	uint32_t hval = hash(key) % (cht->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(cht->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD) {
		if (cht->payloads[offset + counter].key == key) {
			cht_entry entry = cht->payloads[offset + counter];
			scanfunc(entry.key, entry.payload);
		}
		counter++;
	}
	hash_scan(cht->overflow, key, scanfunc);
}

bool cht_has(cht* cht, uint32_t key) {
	return cht_find_uniq(cht, key) != NULL;
}

void cht_free(cht* cht) {
	free(cht->bitmap);
	free(cht->payloads);
	hash_free(cht->overflow);
	free(cht);
}
