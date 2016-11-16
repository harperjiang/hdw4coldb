/*
 * cht.c
 *
 *  Created on: Nov 16, 2016
 *      Author: Hao Jiang
 */

#include <stdlib.h>
#include "cht.h"

#define BITMAP_FACTOR 8
/*
 * Multiplicative hashing
 */
uint32_t hash(uint32_t v) {
	return v * UINT32_C(2654435761);
}

/**
 * Bitmap operations
 */
void bitmap_set(uint64_t* bitmap, uint32_t offset) {

}

uint32_t bitmap_get(uint64_t* bitmap, uint32_t offset) {

}

void bitmap_setpopu(uint64_t* bitmap, uint32_t offset) {

}

uint32_t bitmap_getpopu(uint64_t* bitmap, uint32_t offset) {

}

/*
 * Build a CHT with given data, use two passing
 */
void build(cht* cht, data* datas, uint32_t size) {

	uint32_t bitmap_size = BITMAP_FACTOR * size / 32;

	cht->size = size;
	cht->bitmap = (uint64_t*) malloc(sizeof(uint64_t) * bitmap_size);
	cht->payloads = (data*) malloc(sizeof(data) * size);

	// The first pass, fill in bitmap and payloads array
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hash = hash(datas[i]->key) % bitmap_size;
		bitmap_set(cht->bitmap, hash);
	}
	// The second pass, compute offset in bitmap

}

/**
 * Looking for key in CHT
 */
data* find(cht* cht, uint8_t* key) {

}
