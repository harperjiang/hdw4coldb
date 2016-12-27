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
void remainder(__m256i* hashed, __m256i* index, __m256i* offset, uint big) {
	uint* rem = (uint*)offset;
	uint* quo = (uint*)index;
	uint* as = (uint*)hashed;
	for(uint i = 0; i < 8;i++) {
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
	}
}
/**
 * Check 64-bit bitmap to see if the given key exists in the bitmap
 */
__m256i SimdCHTJoin::check_bitmap(ulong* bitmap, uint bitmapSize,
		__m256i input) {
	uint byteSize = 32;
	__m256i hashed = _mm256_mullo_epi32(input, HASH_FACTOR);
	__m256i offset;
	__m256i index;
	remainder(&hashed, &index, &offset, bitmapSize * BITMAP_UNIT);
	__m256i index2n = _mm256_add_epi32(index, index);
	// Use index to load from bitmap, the scale here is byte, thus load 32 bit integer use scale 4.
	__m256i byte = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);
	// Use offset to create pattern
	__m256i ptn = _mm256_sllv_epi32(SimdHelper::ONE, offset);
	// -1 for selected key, zero for abandoned key
	__m256i selector = _mm256_cmpeq_epi32(
			_mm256_and_si256(
					_mm256_srav_epi32(_mm256_and_si256(byte, ptn), offset),
					SimdHelper::ONE), SimdHelper::ONE);
	__m256i result = _mm256_and_si256(selector, input);
	return result;
}

/**
 * Return the location of the given key in cht payload, -1 if not found
 */
__m256i SimdCHTJoin::lookup_cht(ulong* bitmap, uint bitmapSize,
		uint* chtpayload, uint chtsize, __m256i input, __m256i* remain) {
	__m256i hashed = SimdHelper::remainder_epu32(_mm256_mullo_epi32(input, HASH_FACTOR),
	bitmapSize * BITMAP_UNIT);
	__m256i offset;
	__m256i index = SimdHelper::divrem_epu32(&offset, hashed, BITMAP_UNIT);
	__m256i index2n = _mm256_add_epi32(index, index);
	__m256i index2n1 = _mm256_add_epi32(index2n, SimdHelper::ONE);

	__m256i basePop = _mm256_i32gather_epi32((int* )bitmap, index2n1, 4);
	__m256i loadOffset = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);

	__m256i mask = _mm256_xor_si256(_mm256_sllv_epi32(SimdHelper::MAX, offset), SimdHelper::MAX);
	__m256i partialPop = SimdHelper::popcnt_epi32(_mm256_and_si256(loadOffset, mask));

	__m256i location = _mm256_add_epi32(basePop, partialPop);

	__m256i result = SimdHelper::ZERO;

	__m256i notzero = SimdHelper::testnz_epi32(input);

	for (int i = 0; i < THRESHOLD; i++) {
		__m256i chtval = _mm256_i32gather_epi32((int* )chtpayload, location, 4);
		// a value of -1 means found
		__m256i compare = _mm256_xor_si256(input, chtval);
		__m256i locmask = _mm256_and_si256(notzero, SimdHelper::testz_epi32(compare));
		// Store location + 1 in result, 0 for not found
		__m256i locadd1 = _mm256_add_epi32(location, SimdHelper::ONE);
		__m256i locstore = _mm256_and_si256(locmask, locadd1);
		result = _mm256_or_si256(result, locstore);
		// If location is greater than boundary, reduce it
		location = SimdHelper::remainder_epu32(locadd1, chtsize);

	}

	__m256i resmask = SimdHelper::testz_epi32(result);
	// Location for found key, and -1 for not found
	result = _mm256_sub_epi32(result, SimdHelper::ONE);

	// Remaining key
	__m256i rem = _mm256_and_si256(resmask, input);
	_mm256_store_si256(remain,rem);

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
		__m256i load = _mm256_i32gather_epi32((int* )hashbuckets, hashed, 4);
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
	CHT* cht = (CHT*) _lookup;
	NotEqual nz(0);

	_timer.start();
	ulong* bitmap = cht->bitmap;
	uint bitmapSize = cht->bitmap_size;
	__m256i moffset;
	__m256i mindex;
	for (uint i = 0; i < _probeSize / 8; i++) {
		uint offset = i * 8;
		__m256i input = _mm256_load_si256((__m256i *) (_probe + offset));
		__m256i* storeloc = (__m256i*)(bitmapresult+offset);

		__m256i hashed = _mm256_mullo_epi32(input, HASH_FACTOR);

		remainder(&hashed, &mindex, &moffset, bitmapSize * BITMAP_UNIT);
		__m256i index2n = _mm256_add_epi32(mindex, mindex);
		// Use index to load from bitmap, the scale here is byte, thus load 32 bit integer use scale 4.
		__m256i byte = _mm256_i32gather_epi32((int* )bitmap, index2n, 4);
		// Use offset to create pattern
		__m256i ptn = _mm256_sllv_epi32(SimdHelper::ONE, moffset);
		// -1 for selected key, zero for abandoned key
		__m256i selector = _mm256_cmpeq_epi32(
				_mm256_and_si256(
						_mm256_srav_epi32(_mm256_and_si256(byte, ptn), moffset),
						SimdHelper::ONE), SimdHelper::ONE);
		__m256i result = _mm256_and_si256(selector, input);

		_mm256_store_si256(storeloc, result);
	}
	if (_probeSize % 8) {
		uint psize = _probeSize % 8;
		uint index = (_probeSize / 8) * 8;
		uint* start = _probe + index;
		__m256i loadpartial = _mm256_setr_epi32(psize >= 1 ? start[0] : 0,
				psize >= 2 ? start[1] : 0, psize >= 3 ? start[2] : 0,
				psize >= 4 ? start[3] : 0, psize >= 5 ? start[4] : 0,
				psize >= 6 ? start[5] : 0, psize >= 7 ? start[6] : 0, 0);
		__m256i partialprocessed = check_bitmap(bitmap, bitmapSize,
				loadpartial);
		SimdHelper::store_epu32(bitmapresult, index, partialprocessed, psize);
	}

	_timer.interval("filter");

	uint* chtinput = bitmapresult;
	uint chtinputsize = inner->size;

	if (collectBitmap) {
		chtinput = (uint*) aligned_alloc(32, sizeof(uint) * inner->size);
		chtinputsize = CollectThread::collect(bitmapresult, chtinput,
				inner->size, &nz);
		_logger->info("Pass bitmap :%u\n", chtinputsize);
		uint bfhelp = 0;
		for (uint i = 0; i < _probeSize; i++) {
			if (!cht->bf->test(_probe[i])) {
				bfhelp++;
			}
		}
		_logger->info("Bloom Filter filter out %u\n", bfhelp);
		_timer.interval("cht_input_collect");
	}
	for (uint i = 0; i < chtinputsize; i++) {
		if (cht->has(chtinput[i])) {
			_matched->match(chtinput[i], NULL, NULL);
		}
	}
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

	if (collectBitmap) {
		free(chtinput);
	}
//
//	if (collectCht) {
//		free (cmprshashinput);
//	}
	free(bitmapresult);
//	free (hashinput);
//	free (chtresult);
//	free (hashresult);
}

__m256i CheckBitmapTransform::transform(__m256i input) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::check_bitmap(owner->alignedBitmap, cht->bitmapSize(),
			input);
}

__m256i LookupChtTransform::transform3(__m256i input, __m256i* out) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::lookup_cht(owner->alignedBitmap, cht->bitmapSize(),
	owner->alignedChtload,cht->payload_size, input, out);
}

__m256i LookupHashTransform::transform(__m256i input) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::lookup_hash(owner->alignedHashbkt,
			cht->overflow->bucket_size, input);
}

