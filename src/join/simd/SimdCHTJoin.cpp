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
	bool debug = false;
	uint* inputdata = (uint*)&input;
	for(int i = 0; i < 8; i++) {
//		if(inputdata[i] == 4104995369) {
//			debug = true;
//		}
	}
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
	if(debug) {
		printf("input:\t");
		SimdHelper::print_epu32(input);
		printf("hashed \t");
		SimdHelper::print_epu32(hashed);
		printf("base pop\t");
		SimdHelper::print_epu32(basePop);
		printf("load offset\t");
		SimdHelper::print_epu32(loadOffset);
		printf("partial pop\t");
		SimdHelper::print_epu32(partialPop);
		printf("location:\t");
		SimdHelper::print_epu32(location);
	}
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
		if(debug) {
			printf("Round %u\n",i);
			printf("chtval:\t");
			SimdHelper::print_epu32(chtval);
			printf("compare:\t");
			SimdHelper::print_epu32(compare);
			printf("locmask:\t");
			SimdHelper::print_epu32(locmask);
			printf("locadd1:\t");
			SimdHelper::print_epu32(locadd1);
			printf("locstore:\t");
			SimdHelper::print_epu32(locstore);
			printf("result\t");
			SimdHelper::print_epu32(result);
		}
	}

	__m256i resmask = SimdHelper::testz_epi32(result);
	// Location for found key, and -1 for not found
	result = _mm256_sub_epi32(result, SimdHelper::ONE);

	// Remaining key
	__m256i rem = _mm256_and_si256(resmask, input);
	::memcpy(remain, &rem, 32);
	if(debug) {
		printf("result\t");
		SimdHelper::print_epu32(result);
		printf("remain\t");
		SimdHelper::print_epu32(rem);
	}
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
		_logger->debug("Records passed bitmap check: %u\n", chtinputsize);
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
		_logger->debug("Records to check in hash: %u\n", hashinputsize);
		_timer.interval("hash_input_collect");
	}
	uint* hashresult = new uint[hashinputsize];

	LookupHashTransform lht(this);
	SimdHelper::transform(cmprshashinput, hashinputsize, hashresult, &lht);
	_timer.interval("hash_lookup");

	NotEqual nmax(0xffffffff);
	uint foundInCht = CounterThread::count(chtresult, chtinputsize, &nmax,
			_matched);
	uint foundInHash = CounterThread::count(hashresult, hashinputsize, &nmax,
			_matched);
	// FIX ME Debug info
	_logger->debug("Found in CHT: %u\n", foundInCht);
	_logger->debug("Found in Hash: %u\n", foundInHash);
	CHT* cht = (CHT*) _lookup;

	for (uint i = 0; i < chtinputsize; i++) {
		if (cht->has(chtinput[i]) && chtresult[i] == 0xffffffff
				&& hashresult[i] == 0xffffffff) {
			_logger->warn("Key mismatch %u,%u,%u,%u\n", chtinput[i],
					chtresult[i], hashresult[i],
					cht->overflow->hash(chtinput[i]));
		}
	}

	uint incht = 0;
	uint inhash = 0;
	for (uint i = 0; i < _probeSize; i++) {
		if (cht->has(_probe[i])) {
			if (cht->overflow->has(_probe[i])) {
				inhash++;
			} else {
				incht++;
			}
		}
	}
	_logger->debug("Real in CHT: %u\n", incht);
	_logger->debug("Real in Hash: %u\n", inhash);

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

	__m256i result = SimdCHTJoin::lookup_cht(owner->alignedBitmap, cht->bitmapSize(),
	owner->alignedChtload,cht->payload_size, input, out);

	Logger* logger = owner->_logger;
	uint* inputdata = (uint*)&input;
	uint* resultdata = (uint*)&result;
	uint* outdata = (uint*)out;

	for(uint i = 0; i < 8; i++) {
		uint key = inputdata[i];
		if(cht->has(key)) {
			if(cht->overflow->has(key)) {
				// in overflow
				if(resultdata[i] != 0xffffffff) {
					logger->warn("Result for key %u, should be in overflow, now is found at %u\n",key,resultdata[i]);
				}
				if(outdata[i] != key) {
					logger->warn("Result for key %u, should be in overflow, now not going with %u\n",key,outdata[i]);
				}
			} else {
				// in cht
				if(resultdata[i] == 0xffffffff) {
					logger->warn("Result for key %u, should be found, now not\n",key);
				}
				if(outdata[i] != 0) {
					logger->warn("Result for key %u, should not go to overflow, now go %u\n",key,outdata[i]);
				}
			}
		} else {
			if(resultdata[i] != 0xffffffff) {
				logger->warn("Result for key %u, should be not found and go to overflow, now %u\n",key,resultdata[i]);
			}
			if(outdata[i] != key) {
				logger->warn("Result for key %u, should be not found and go to overflow, now not go %u\n",key, outdata[i]);
			}
		}
	}

	return result;
}

__m256i LookupHashTransform::transform(__m256i input) {
	CHT* cht = (CHT*) owner->_lookup;
	return SimdCHTJoin::lookup_hash(owner->alignedHashbkt,
			cht->overflow->bucket_size, input);
}

__m256i AndTransform::transform2(__m256i a, __m256i b) {
	return _mm256_and_si256(a, b);
}
