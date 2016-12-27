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

// index = hashed % big / 32
// offset = hashed % big % 32
__m256i remainder(__m256i* hashed, uint big) {
	uint* as = (uint*)hashed;
	for(uint i = 0; i < 8;i++) {
		as[i]%=big;
		/*
		 asm volatile(
		 "xorl %%edx, %%edx\n\t"
		 "movl %2,%%eax\n\t"
		 "divl %3\n\t"
		 "movl %%edx,%%eax\n\t"
		 "shrl $5, %%edx\n\t"
		 "andl $31, %%eax\n\t"
		 "movl %%eax,%1\n\t"
		 "movl %%edx,%0\n\t"
		 :"=m"(quo[i]),"=m"(rem[i])
		 :"m"(as[i]),"r"(big)
		 :"edx","eax");
		 */
	}
	return _mm256_load_si256(hashed);
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
	CHT* cht = (CHT*) _lookup;

	_timer.start();
	ulong* bitmap = cht->bitmap;
	uint bitmapSize = cht->bitmap_size;

	uint store_offset = 0;
	for (uint i = 0; i < _probeSize / 8; i++) {
		uint load_offset = i * 8;
		__m256i input = _mm256_load_si256((__m256i *) (_probe + load_offset));
		__m256i* storeloc = (__m256i*)(bitmapresult+store_offset);

		__m256i hashed = _mm256_mullo_epi32(input, HASH_FACTOR);
		hashed = remainder(&hashed, bitmapSize * BITMAP_UNIT);

		__m256i index = _mm256_srli_epi32(hashed, 5);
//		SimdHelper::print_epu32(index);
		__m256i offset = _mm256_and_si256(hashed, SimdHelper::THIRTY_ONE);
		__m256i index2n = _mm256_add_epi32(index, index);
		__m256i index2n1 = _mm256_add_epi32(index2n, SimdHelper::ONE);
		// Use index to load from bitmap, the scale here is byte, thus load 32 bit integer use scale 4.
		__m256i byte = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);
		__m256i basePop = _mm256_i32gather_epi32((int* )bitmap, index2n1, 4);

		// Use offset to create pattern
		__m256i ptn = _mm256_sllv_epi32(SimdHelper::ONE, offset);
		__m256i mask = _mm256_xor_si256(
				_mm256_cmpeq_epi32(_mm256_and_si256(byte, ptn),
						SimdHelper::ZERO), SimdHelper::MAX);

		__m256i popmask = _mm256_xor_si256(
				_mm256_sllv_epi32(SimdHelper::MAX, offset), SimdHelper::MAX);
		__m256i partialPop = SimdHelper::popcnt_epi32(
				_mm256_and_si256(byte, popmask));

		__m256i location = _mm256_and_si256(
				_mm256_add_epi32(basePop, partialPop), mask);
		if (!_mm256_testz_si256(location, SimdHelper::MAX)) {
			_mm256_store_si256(storeloc, location);
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
//		__m256i partialprocessed = check_bitmap(bitmap, bitmapSize,
//				loadpartial);
		// TODO Fix this
		__m256i partialprocessed = loadpartial;
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

