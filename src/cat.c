/*
 * cat.c
 *
 *  Created on: Nov 19, 2016
 *      Author: harper
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <memory.h>
#include "cat.h"
#include "util.h"

#define CAT_BITMAP_FACTOR 	1
#define CAT_BITMAP_UNIT 32

extern bool bitmap_testset(uint64_t* bitmap, uint32_t offset);
extern bool bitmap_test(uint64_t* bitmap, uint32_t offset);
extern void bitmap_setpopcnt(uint64_t* bitmap, uint32_t offset, uint32_t value);
extern uint32_t bitmap_popcnt(uint64_t* bitmap, uint32_t hval);

void cat_build(cat* cat, kv* datas, uint32_t size) {
	// In a CAT, the bitmap size is determined by key ranges.
	cat->max = 0;
	cat->min = 0xffffffff;
	for (uint32_t i = 0; i < size; i++) {
		cat->max = cat->max > datas[i].key ? cat->max : datas[i].key;
		cat->min = cat->min < datas[i].key ? cat->min : datas[i].key;
	}

	uint32_t bitsize = cat->max - cat->min + 1;

	uint32_t bitmap_size = bitsize / CAT_BITMAP_UNIT;
	if (bitmap_size * CAT_BITMAP_UNIT < bitsize) {
		bitmap_size++;
	}

	cat->bitmap = (uint64_t*) calloc(sizeof(uint64_t), bitmap_size);
	cat->bitmap_size = bitmap_size;

	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = (datas[i].key - cat->min);
		assert(bitmap_testset(cat->bitmap, hval));
	}

	uint32_t sum = 0;
	for (uint32_t i = 0; i < bitmap_size; i++) {
		bitmap_setpopcnt(cat->bitmap, i, sum);
		sum += popcount(cat->bitmap[i]);
	}

	cat->payload_size = sum;
	cat->payloads = (uint8_t*) malloc(sizeof(uint8_t) * sum * PAYLOAD_SIZE);

	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = datas[i].key - cat->min;
		uint32_t offset = bitmap_popcnt(cat->bitmap, hval);
		memcpy(cat->payloads + PAYLOAD_SIZE * offset, datas[i].payload,
		PAYLOAD_SIZE);
	}
}

bool cat_has(cat* cat, uint32_t key) {
	if (key > cat->max || key < cat->min)
		return false;
	return bitmap_test(cat->bitmap, key - cat->min);
}

uint8_t* cat_find_uniq(cat* cat, uint32_t key) {
	uint32_t hval = key - cat->min;
	if (bitmap_test(cat->bitmap, hval)) {
		uint32_t offset = bitmap_popcnt(cat->bitmap, hval);
		return cat->payloads + offset * PAYLOAD_SIZE;
	}
	return NULL ;
}

void cat_free(cat* cat) {
	free(cat->bitmap);
	free(cat->payloads);
	free(cat);
}
