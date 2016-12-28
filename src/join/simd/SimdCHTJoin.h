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

protected:
	static __m256i HASH_FACTOR;
	static __m256i PERMUTE;
	static __m256i SHIFT_MASK_LOW;
	static __m256i SHIFT_MASK_HIGH;
	__m256i bitsize;

	// Functions for internal use
	__m256i process(__m256i input);
	void load_bitmap(__m256i index, __m256i* base, __m256i* byte);
public:
	SimdCHTJoin(bool = false, bool = false, bool = false);
	virtual ~SimdCHTJoin();

	void join(kvlist* outer, kvlist* inner);
};

#endif /* SRC_JOIN_SIMDCHTJOIN_H_ */
