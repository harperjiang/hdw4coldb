/*
 * SimdCHTJoin.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "SimdCHTJoin.h"

#include <avx2intrin.h>
#include <avxintrin.h>
#include <stdlib.h>
#include <sys/types.h>
#include <cstdint>
#include <cstring>

#include "../../lookup/Hash.h"
#include "../../lookup/Lookup.h"
#include "../../util/Logger.h"
#include "../CollectThread.h"
#include "../CounterThread.h"
#include "../Predicate.h"

#define BITMAP_UNIT 32
#define THRESHOLD 5

__m256i SimdCHTJoin::HASH_FACTOR = _mm256_set1_epi32(
		(int) UINT32_C(2654435761));

/**
 * Check 64-bit bitmap to see if the given key exists in the bitmap
 */
__m256i SimdCHTJoin::check_bitmap(ulong* bitmap, uint bitmapSize,
		__m256i input) {
	uint byteSize = 32;

	__m256i hashed = SimdHelper::remainder_epu32(
			_mm256_mullo_epi32(input, HASH_FACTOR), bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = SimdHelper::divrem_epu32(&offset, hashed, byteSize);
	__m256i index2n = _mm256_add_epi32(index, index);

	// Use index to load from bitmap, the scale here is byte, thus load 32 bit integer use scale 4.
	__m256i byte = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);
	// Use offset to create pattern
	__m256i ptn = _mm256_sllv_epi32(SimdHelper::ONE, offset);
	// -1 for selected key, zero for abandoned key
	__m256i selector = _mm256_srav_epi32(_mm256_and_si256(byte, ptn), offset);
	selector = _mm256_sign_epi32(selector, SimdHelper::MAX);
	return _mm256_and_si256(selector, input);
}

/**
 * Return the location of the given key in cht payload, -1 if not found
 */
__m256i SimdCHTJoin::lookup_cht(ulong* bitmap, uint bitmapSize,
		uint* chtpayload, uint chtsize, __m256i input, __m256i* remain) {
	__m256i hashed = SimdHelper::remainder_epu32(
	_mm256_mullo_epi32(input, HASH_FACTOR), bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = SimdHelper::divrem_epu32(&offset, hashed, BITMAP_UNIT);
	__m256i index2n = _mm256_add_epi32(index, index);
	__m256i index2n1 = _mm256_add_epi32(index2n, SimdHelper::ONE);

	__m256i basePop = _mm256_i32gather_epi32((int* )bitmap, index2n1, 4);
	__m256i loadOffset = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);

	__m256i mask = _mm256_xor_si256(_mm256_srav_epi32(SimdHelper::MAX, offset),
	SimdHelper::MAX);
	__m256i partialPop = SimdHelper::popcnt_epi32(
	_mm256_and_si256(loadOffset, mask));

	__m256i location = _mm256_add_epi32(basePop, partialPop);

	__m256i result = SimdHelper::ZERO;

	for (int i = 0; i < THRESHOLD; i++) {
		__m256i chtval = _mm256_i32gather_epi32((int* )chtpayload, location, 4);
		// a value of 0 means found
		__m256i compare = _mm256_xor_si256(input, chtval);
		// Test function is not available in AVX2, write my own
		__m256i locmask = SimdHelper::testz_epi32(compare);
		// Store location + 1 in result, 0 for not found
		__m256i locstore = _mm256_and_si256(locmask,
		_mm256_add_epi32(location, SimdHelper::ONE));
		result = _mm256_or_si256(result, locstore);
		location = _mm256_add_epi32(location, SimdHelper::ONE);
		// If location is greater than boundary, reduce it
		location = SimdHelper::remainder_epu32(location, chtsize);
	}

	__m256i resmask = SimdHelper::testz_epi32(result);
	// Location for found key, and -1 for not found
	result = _mm256_sub_epi32(result, SimdHelper::ONE);

	// Remaining key
	__m256i rem = _mm256_and_si256(resmask, input);
	::memcpy(remain, &rem, 32);
	return result;
}

__m256i SimdCHTJoin::lookup_hash(uint* hashbuckets, uint bktsize,
		__m256i input) {
	__m256i hashed = SimdHelper::remainder_epu32(
			_mm256_mullo_epi32(input, HASH_FACTOR), bktsize);
	__m256i inputnz = SimdHelper::testnz_epi32(input);
	__m256i flag = input;
	__m256i result = SimdHelper::ZERO;
	while (!_mm256_testz_si256(flag, SimdHelper::MAX)) {
		__m256i load = _mm256_i32gather_epi32(hashbuckets, hashed, 4);
		// For key equal to load and nz, store location
		__m256i locmask = _mm256_and_si256(inputnz,
				SimdHelper::testz_epi32(_mm256_xor_si256(load, input)));
		__m256i hashed1 = _mm256_add_epi32(hashed, SimdHelper::ONE);
		result = _mm256_or_si256(result, _mm256_and_si256(locmask, hashed1));
		// For 0, store 0 to flag
		__m256i flagmask = SimdHelper::testnz_epi32(load);
		flag = _mm256_and_si256(flagmask, flag);
		// For found, store 0 to flag
		flag = _mm256_and_si256(flag,
				_mm256_sub_epi32(SimdHelper::ZERO,
						_mm256_add_epi32(SimdHelper::ONE, locmask)));
		// Increase by one
		hashed = SimdHelper::remainder_epu32(hashed1, bktsize);
	}
	return _mm256_sub_epi32(result, SimdHelper::ONE);
}

SimdCHTJoin::SimdCHTJoin(bool c1, bool c2, bool ep) :
		Join(ep) {
	this->collectBitmap = c1;
	this->collectCht = c2;
	this->_logger = Logger::getLogger("SimdCHTJoin");
}

SimdCHTJoin::~SimdCHTJoin() {
	if (NULL != _probe)
		::free(_probe);
	_probe = NULL;
	if (alignedBitmap != NULL)
		::free(alignedBitmap);
	if (alignedChtload != NULL)
		::free(alignedChtload);
	if (alignedHashbkt != NULL)
		::free(alignedHashbkt);
}

Lookup* SimdCHTJoin::createLookup() {
	return new CHT();
}

const char* SimdCHTJoin::name() {
	return "SimdCHTJoin";
}

void SimdCHTJoin::buildLookup(kvlist* outer) {
	Join::buildLookup(outer);

	CHT* cht = (CHT*) _lookup;
	alignedBitmap = (ulong*) ::aligned_alloc(32,
			sizeof(ulong) * cht->bitmap_size);
	::memcpy(alignedBitmap, cht->bitmap, sizeof(ulong) * cht->bitmap_size);

	alignedChtload = (uint*) ::aligned_alloc(32,
			sizeof(uint) * cht->payload_size);
	::memcpy(alignedChtload, cht->keys, sizeof(uint) * cht->payload_size);

	alignedHashbkt = (uint*) ::aligned_alloc(32,
			sizeof(uint) * cht->overflow->bucket_size);
	::memcpy(alignedHashbkt, cht->overflow->buckets,
			sizeof(uint) * cht->overflow->bucket_size);
}

void SimdCHTJoin::buildProbe(kvlist* inner) {
	_probe = (uint*) aligned_alloc(32, inner->size * sizeof(uint));
	for (uint i = 0; i < inner->size; i++) {
		_probe[i] = inner->entries[i].key;
	}
	_probeSize = inner->size;
}

void SimdCHTJoin::join(kvlist* outer, kvlist* inner) {
	// Allocate aligned bitmap

	buildLookup(outer);
	buildProbe(inner);

	uint* bitmapresult = (uint*) aligned_alloc(32, sizeof(uint) * _probeSize);

	NotEqual nz(0);

	_timer.start();

	CheckBitmapTransform cbt(this);
	SimdHelper::transform(_probe, _probeSize, bitmapresult, &cbt);

	_timer.interval("filter");

	uint* chtinput = bitmapresult;
	uint chtinputsize = inner->size;

	if (collectBitmap) {
		chtinput = (uint*) aligned_alloc(32, sizeof(uint) * inner->size);
		chtinputsize = CollectThread::collect(bitmapresult, chtinput,
				inner->size, &nz);
		_timer.interval("cht_input_collect");
	}
	uint* chtresult = new uint[chtinputsize];
	uint* hashinput = (uint*) aligned_alloc(32, sizeof(uint) * chtinputsize);

	LookupChtTransform lct(this);
	SimdHelper::transform3(chtinput, chtinputsize, chtresult, hashinput, &lct);
	_timer.interval("cht_lookup");

	uint* cmprshashinput = hashinput;
	uint hashinputsize = chtinputsize;
	if (collectCht) {
		cmprshashinput = (uint*) aligned_alloc(32, sizeof(uint) * chtinputsize);
		hashinputsize = CollectThread::collect(hashinput, cmprshashinput,
				chtinputsize, &nz);
		_timer.interval("hash_input_collect");
	}
	uint* hashresult = new uint[hashinputsize];

	LookupHashTransform lht(this);
	SimdHelper::transform(cmprshashinput, hashinputsize, hashresult, &lht);
	_timer.interval("hash_lookup");

	NotEqual nmax(0xffffffff);
	uint matched = CounterThread::count(chtresult, chtinputsize, &nmax);
	matched += CounterThread::count(hashresult, hashinputsize, &nmax, _matched);
	_timer.interval("count_result");

	_timer.stop();

	printSummary();

	if (collectBitmap) {
		free(chtinput);
	}
	if (collectCht) {
		free(cmprshashinput);
	}
	free(bitmapresult);
	free(hashinput);
	delete[] chtresult;
	delete[] hashresult;
	delete _lookup;
}

__m256i CheckBitmapTransform::transform(__m256i input) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::check_bitmap(owner->alignedBitmap, cht->bitmapSize(),
			input);
}

__m256i LookupChtTransform::transform3(__m256i input, __m256i* out) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::lookup_cht(owner->alignedBitmap,
	cht->bitmapSize(), owner->alignedChtload,
	cht->payload_size, input, out);
}

__m256i LookupHashTransform::transform(__m256i input) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::lookup_hash(owner->alignedHashbkt,
			cht->overflow->bucket_size, input);
}

__m256i AndTransform::transform2(__m256i a, __m256i b) {
	return _mm256_and_si256(a, b);
}
