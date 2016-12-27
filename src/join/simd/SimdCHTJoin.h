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

};

#endif /* SRC_JOIN_SIMDCHTJOIN_H_ */
