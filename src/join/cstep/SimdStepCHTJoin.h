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

class SimdStepCHTJoin: public Join {
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

	SimdStepCHTJoin(bool = false, bool = false, bool = false);
	virtual ~SimdStepCHTJoin();

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
	SimdStepCHTJoin* owner;
public:
	SCheckBitmapTransform(SimdStepCHTJoin* css) {
		owner = css;
	}
	virtual ~SCheckBitmapTransform() {
	}

	__m256i transform(__m256i);
};

class SLookupChtTransform: public SimdTransform {
private:
	SimdStepCHTJoin* owner;
public:
	SLookupChtTransform(SimdStepCHTJoin* css) {
		owner = css;
	}
	virtual ~SLookupChtTransform() {
	}
	__m256i transform3(__m256i, __m256i *);
};

class SLookupHashTransform: public SimdTransform {
private:
	SimdStepCHTJoin* owner;
public:
	SLookupHashTransform(SimdStepCHTJoin* css) {
		owner = css;
	}
	virtual ~SLookupHashTransform() {
	}
	__m256i transform(__m256i);
};

#endif /* SRC_JOIN_SIMDCHTJOIN_H_ */
