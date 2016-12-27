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
#include "CHTConstants.h"
#include "LookupHelper.h"

using namespace std;

/**===========================================================================
 * Bitmap operations
 =============================================================================*/

/**
 * Test a bitmap location and set it to 1 if it is 0, return 1 if set
 */
bool bitmap_testset(uint64_t* bitmap, uint32_t offset) {
	/*bool result;
	 asm("xorq %%rax,%%rax\n\t"
	 "movl %1,%%edx\n\t"
	 "movl %%edx,%%eax\n\t"
	 "shrl $5,%%eax\n\t"
	 "andl $31,%%edx\n\t"
	 "btsl %%edx, (%2,%%rax,8)\n\t"
	 "setnc %0\n\t"
	 :"=m"(result)
	 :"m"(offset),"r"(bitmap)
	 :"cc","edx","rax"
	 );
	 return result;*/
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
	/*bool result;
	 asm("xorq %%rax,%%rax\n\t"
	 "movl %1,%%edx\n\t"
	 "movl %%edx,%%eax\n\t"
	 "shrl $5,%%eax\n\t"
	 "andl $31,%%edx\n\t"
	 "btl %%edx, (%2,%%rax,8)\n\t"
	 "setc %0\n\t"
	 :"=m"(result)
	 :"m"(offset),"r"(bitmap)
	 :"cc","edx","rax"
	 );
	 return result;
	 */
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

uint bfdata[3] = { 3224232347, 1826503991, 114141513 };

CHT::CHT() :
		Lookup("CHT") {
	bitmap = NULL;
	bitmap_size = 0;
	keys = NULL;
	payloads = NULL;
	payload_size = 0;
	overflow = NULL;
	bf = new BloomFilter(3, bfdata);
}

CHT::~CHT() {
	if (NULL != bitmap)
		delete[] bitmap;
	if (NULL != keys)
		delete[] keys;
	if (NULL != payloads)
		delete[] payloads;
	if (NULL != overflow)
		delete overflow;
	delete bf;
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
	this->bf->build(entries, size);

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
	this->keys = new uint32_t[sum]();
	this->payloads = new uint8_t[sum * PAYLOAD_SIZE];
	for (uint32_t i = 0; i < size; i++) {
		uint32_t hval = mut_hash(entries[i].key) % bitsize;
		uint32_t item_offset = bitmap_popcnt(this->bitmap, hval);

		for (uint32_t counter = 0; counter < THRESHOLD; counter++) {
			if (keys[item_offset + counter] == 0) {
				keys[item_offset + counter] = entries[i].key;
				::memcpy(payloads + (item_offset + counter) * PAYLOAD_SIZE,
						entries[i].payload, sizeof(uint8_t) * PAYLOAD_SIZE);
				break;
			}
		}
	}
	overflow->organize(overflow->size() * RATIO);
}

bool CHT::has(uint32_t key) {
	return NULL != this->findUnique(key);
}

uint32_t CHT::payloadSize() {
	return this->payload_size;
}

uint32_t CHT::bitmapSize() {
	return this->bitmap_size;
}

uint8_t* CHT::findUnique(uint32_t key) {
	uint32_t hval = mut_hash(key) % (bitmap_size * BITMAP_UNIT);
	if (!bitmap_test(bitmap, hval)) {
		return NULL;
	}
	if (!bf->test(key)) {
		return NULL;
	}
	uint32_t offset = bitmap_popcnt(this->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD && offset + counter < payload_size
			&& this->keys[offset + counter] != key) {
		counter++;
	}
	if (counter == THRESHOLD || offset + counter >= payload_size) {
		return this->overflow->get(key);
	}

	return this->payloads + PAYLOAD_SIZE * (offset + counter);
}

uint8_t* CHT::access(uint32_t key) {
	return this->findUnique(key);
}

void CHT::scan(uint32_t key, ScanContext* context) {
	if (!has(key))
		return;
	uint32_t hval = mut_hash(key) % (this->bitmap_size * BITMAP_UNIT);
	uint32_t offset = bitmap_popcnt(this->bitmap, hval);

	uint32_t counter = 0;
	while (counter < THRESHOLD) {
		if (this->keys[offset + counter] == key) {
			context->execute(key,
					this->payloads + PAYLOAD_SIZE * (offset + counter));
		}
		counter++;
	}
	this->overflow->scan(key, context);
}

Hash * CHT::getOverflow() {
	return overflow;
}
