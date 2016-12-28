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
#include <stdio.h>
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
__m256i SimdCHTJoin::PERMUTE = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
__m256i SimdCHTJoin::SHIFT_MASK_LOW = _mm256_set1_epi64x(0xFFFFFFFF);
__m256i SimdCHTJoin::SHIFT_MASK_HIGH = _mm256_set1_epi64x(0xFFFFFFFF00000000);

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

	bitsize = _mm256_set1_epi32(cht->bitmap_size * BITMAP_UNIT - 1);
}

void SimdCHTJoin::buildProbe(kvlist* inner) {
	_probe = (uint*) aligned_alloc(32, inner->size * sizeof(uint));
	for (uint i = 0; i < inner->size; i++) {
		_probe[i] = inner->entries[i].key;
	}
	_probeSize = inner->size;
}

__m256i SimdCHTJoin::process(__m256i input) {
	__m256i hashed = _mm256_and_si256(_mm256_mullo_epi32(input, HASH_FACTOR),
			bitsize);
	__m256i index = _mm256_srli_epi32(hashed, 5);
	__m256i offset = _mm256_and_si256(hashed, SimdHelper::THIRTY_ONE);

	// Load 8 64-bit words
	__m256i permute = _mm256_permutevar8x32_epi32(index, PERMUTE);
	__m128i index1 = _mm256_extracti128_si256(permute, 0);
	__m128i index2 = _mm256_extracti128_si256(permute, 1);
	// Each load has 4 64 bits
	__m256i load1 = _mm256_i32gather_epi64((const long long int* )alignedBitmap,
			index1, 8);
	__m256i load2 = _mm256_i32gather_epi64((const long long int* )alignedBitmap,
			index2, 8);
	// lower 32
	__m256i byte = _mm256_or_si256(_mm256_and_si256(load1, SHIFT_MASK_LOW),
			_mm256_slli_epi64(_mm256_and_si256(load2, SHIFT_MASK_LOW), 32));
	// higher 32
	__m256i basePop = _mm256_or_si256(
			_mm256_srli_epi64(_mm256_and_si256(load1, SHIFT_MASK_HIGH), 32),
			_mm256_and_si256(load2, SHIFT_MASK_HIGH));

	// Use offset to create pattern
	__m256i ptn = _mm256_sllv_epi32(SimdHelper::ONE, offset);
	// -1 for passed bitmap check, 0 for failed
	__m256i bitmap_mask = _mm256_xor_si256(
			_mm256_cmpeq_epi32(_mm256_and_si256(byte, ptn), SimdHelper::ZERO),
			SimdHelper::MAX);
	__m256i popmask = _mm256_xor_si256(
			_mm256_sllv_epi32(SimdHelper::MAX, offset), SimdHelper::MAX);
	__m256i partialPop = SimdHelper::popcnt_epi32(
			_mm256_and_si256(byte, popmask));

	__m256i location = _mm256_and_si256(_mm256_add_epi32(basePop, partialPop),
			bitmap_mask);

	// Return not found
	if (_mm256_testz_si256(bitmap_mask, SimdHelper::MAX)) {
		return SimdHelper::MAX;
	}
	// Check cht

	__m256i chtFound = SimdHelper::ZERO;
	for (int i = 0; i < THRESHOLD; i++) {
		if (_mm256_testz_si256(location, SimdHelper::MAX))
			break;
		// Compare data at location with key
		__m256i gathered = _mm256_i32gather_epi32((int* )alignedChtload,
				location, 4);
		__m256i foundmask = _mm256_cmpeq_epi32(gathered, input);
		__m256i donemask = _mm256_xor_si256(foundmask, SimdHelper::MAX);
		__m256i locationadd1 = _mm256_add_epi32(location, SimdHelper::ONE);
		chtFound = _mm256_or_si256(chtFound,
				_mm256_and_si256(locationadd1, foundmask));
		location = _mm256_and_si256(locationadd1, donemask);
	}

	return chtFound;
}

void SimdCHTJoin::join(kvlist* outer, kvlist* inner) {
// Allocate aligned bitmap

	buildLookup(outer);
	buildProbe(inner);

	uint* bitmapresult = (uint*) aligned_alloc(32, sizeof(uint) * _probeSize);

	_timer.start();

	uint store_offset = 0;
	for (uint i = 0; i < _probeSize / 8; i++) {
		uint load_offset = i * 8;
		__m256i input = _mm256_load_si256((__m256i *) (_probe + load_offset));
		__m256i* storeloc = (__m256i*)(bitmapresult+store_offset);
		__m256i processed = process(input);
		if (!_mm256_testz_si256(processed, SimdHelper::MAX)) {
			_mm256_store_si256(storeloc, processed);
			store_offset += 8;
		}
	}
	if (_probeSize % 8) {
		uint psize = _probeSize % 8;
		uint index = (_probeSize / 8) * 8;
		uint* start = _probe + index;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		__m256i partialprocessed = process(loadpartial);
		SimdHelper::store_epu32(bitmapresult, store_offset, partialprocessed,
				psize);
	}

	_timer.interval("filter");

//	uint* chtinput = bitmapresult;
//	uint chtinputsize = inner->size;

//	if (collectBitmap) {
//		chtinput = (uint*) aligned_alloc(32, sizeof(uint) * inner->size);
//		chtinputsize = CollectThread::collect(bitmapresult, chtinput,
//				inner->size, &nz);
//		_logger->info("Pass bitmap :%u\n", chtinputsize);
//		_timer.interval("cht_input_collect");
//	}
//	for (uint i = 0; i < chtinputsize; i++) {
//		if (cht->has(chtinput[i])) {
//			_matched->match(chtinput[i], NULL, NULL);
//		}
//	}
	/*
	 uint* chtresult = (uint*) aligned_alloc(32, sizeof(uint) * chtinputsize);
	 uint* hashinput = (uint*) aligned_alloc(32, sizeof(uint) * chtinputsize);

	 LookupChtTransform lct(this);
	 SimdHelper::transform3(chtinput, chtinputsize, chtresult, hashinput, &lct,
	 this->enableProfiling);
	 _timer.interval("cht_lookup");

	 uint* cmprshashinput = hashinput;
	 uint hashinputsize = chtinputsize;
	 if (collectCht) {
	 cmprshashinput = (uint*) aligned_alloc(32, sizeof(uint) * chtinputsize);
	 hashinputsize = CollectThread::collect(hashinput, cmprshashinput,
	 chtinputsize, &nz);
	 _timer.interval("hash_input_collect");
	 }
	 uint* hashresult = (uint*) aligned_alloc(32, sizeof(uint) * hashinputsize);

	 LookupHashTransform lht(this);
	 SimdHelper::transform(cmprshashinput, hashinputsize, hashresult, &lht,
	 this->enableProfiling);
	 _timer.interval("hash_lookup");

	 NotEqual nmax(0xffffffff);
	 CounterThread::count(chtresult, chtinputsize, &nmax, _matched);
	 CounterThread::count(hashresult, hashinputsize, &nmax, _matched);
	 _timer.interval("count_result");
	 */
	_timer.stop();

	printSummary();

//	if (collectBitmap) {
//		free(chtinput);
//	}
//
//	if (collectCht) {
//		free (cmprshashinput);
//	}
	free(bitmapresult);
//	free (hashinput);
//	free (chtresult);
//	free (hashresult);
}

void SimdCHTJoin::load_bitmap(__m256i input, __m256i* base, __m256i* byte) {
	__m256i permute = _mm256_permutevar8x32_epi32(input, PERMUTE);
	__m128i index1 = _mm256_extracti128_si256(permute, 0);
	__m128i index2 = _mm256_extracti128_si256(permute, 1);
	__m256i load1 = _mm256_i32gather_epi64((const long long int* )alignedBitmap,index1, 8);
	__m256i load2 = _mm256_i32gather_epi64((const long long int* )alignedBitmap, index2, 8);
	// lower 32
	*byte = _mm256_or_si256(_mm256_and_si256(load1, SHIFT_MASK_LOW),_mm256_slli_epi64(_mm256_and_si256(load2, SHIFT_MASK_LOW), 32));
	// higher 32
	*base = _mm256_or_si256(_mm256_srli_epi64(_mm256_and_si256(load1, SHIFT_MASK_HIGH), 32), _mm256_and_si256(load2, SHIFT_MASK_HIGH));
}

