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

class SimdCHTJoin: public Join {
	friend class CheckBitmapTransform;
	friend class LookupChtTransform;
	friend class LookupHashTransform;
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

	SimdCHTJoin(bool = false, bool = false, bool = false);
	virtual ~SimdCHTJoin();

	void join(kvlist* outer, kvlist* inner);

public:
	static __m256i HASH_FACTOR;
	static __m256i check_bitmap(ulong* bitmap, uint bitmapSize, __m256i input,
			__m256i* output = NULL);
	static __m256i lookup_cht(ulong* bitmap, uint bitmapSize, uint* chtpayload,
	uint chtsize, __m256i input, __m256i * out);
	static __m256i lookup_hash(uint* hashbuckets, uint bktsize, __m256i input);
};

class CheckBitmapTransform: public SimdTransform {
private:
	SimdCHTJoin* owner;
public:
	CheckBitmapTransform(SimdCHTJoin* css) {
		owner = css;
	}
	virtual ~CheckBitmapTransform() {
	}

	__m256i transform(__m256i);
	void transformv2(__m256i, __m256i *);
};

class LookupChtTransform: public SimdTransform {
private:
	SimdCHTJoin* owner;
public:
	LookupChtTransform(SimdCHTJoin* css) {
		owner = css;
	}
	virtual ~LookupChtTransform() {
	}
	__m256i transform3(__m256i, __m256i *);
};

class LookupHashTransform: public SimdTransform {
private:
	SimdCHTJoin* owner;
public:
	LookupHashTransform(SimdCHTJoin* css) {
		owner = css;
	}
	virtual ~LookupHashTransform() {
	}
	__m256i transform(__m256i);
};

class AndTransform: public SimdTransform {
public:
	AndTransform() {

	}
	virtual ~AndTransform() {

	}
	__m256i transform2(__m256i a, __m256i b);
};

#endif /* SRC_JOIN_SIMDCHTJOIN_H_ */
