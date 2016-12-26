/*
 * SimdCHTJoin.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_SIMDCHTJOIN_H_
#define SRC_JOIN_SIMDCHTJOIN_H_

#include <immintrin.h>
#include "../Join.h"
#include "../../simd/SimdHelper.h"
#include "../../lookup/CHT.h"

class StepSimdCHTJoin: public Join {
	friend class SCheckBitmapTransform;
	friend class SLookupChtTransform;
	friend class SLookupHashTransform;
protected:
	ulong* alignedBitmap = NULL;
	uint* alignedChtload = NULL;
	uint* alignedHashbkt = NULL;

	bool collectBitmap;
	bool collectCht;
protected:
	Lookup* createLookup();
	void buildLookup(kvlist* outer);
	void buildProbe(kvlist* input);

	const char* name();
public:

	StepSimdCHTJoin(bool = false, bool = false, bool = false);
	virtual ~StepSimdCHTJoin();

	void join(kvlist* outer, kvlist* inner);

public:
	static __m256i HASH_FACTOR;
	static __m256i check_bitmap(ulong* bitmap, uint bitmapSize, __m256i input);
	static __m256i lookup_cht(ulong* bitmap, uint bitmapSize, uint* chtpayload,
			uint chtsize, __m256i input, __m256i * out);
	static __m256i lookup_hash(uint* hashbuckets, uint bktsize, __m256i input);
};

class SCheckBitmapTransform: public SimdTransform {
private:
	StepSimdCHTJoin* owner;
public:
	SCheckBitmapTransform(StepSimdCHTJoin* css) {
		owner = css;
	}
	virtual ~SCheckBitmapTransform() {
	}

	__m256i transform(__m256i);
};

class SLookupChtTransform: public SimdTransform {
private:
	StepSimdCHTJoin* owner;
public:
	SLookupChtTransform(StepSimdCHTJoin* css) {
		owner = css;
	}
	virtual ~SLookupChtTransform() {
	}
	__m256i transform3(__m256i, __m256i *);
};

class SLookupHashTransform: public SimdTransform {
private:
	StepSimdCHTJoin* owner;
public:
	SLookupHashTransform(StepSimdCHTJoin* css) {
		owner = css;
	}
	virtual ~SLookupHashTransform() {
	}
	__m256i transform(__m256i);
};

#endif /* SRC_JOIN_SIMDCHTJOIN_H_ */
