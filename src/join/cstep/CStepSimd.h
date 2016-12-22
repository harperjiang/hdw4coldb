/*
 * CStepSimd.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_CSTEPSIMD_H_
#define SRC_JOIN_CSTEP_CSTEPSIMD_H_

#include <immintrin.h>
#include "CStep.h"


class CStepSimd: public CStep {
protected:
	uint* pattern;
	ulong* alignedBitmap;
	uint* alignedChtload;

public:
	static __m256i HASH_FACTOR;
	static __m256i ZERO;
	static __m256i ONE;
	static __m256i TWO;
	static __m256i MAX;

public:
	CStepSimd();
	virtual ~CStepSimd();

	void buildProbe(kvlist* inner);

	void init();
	uint filter(uint* gathered);
	uint lookup(uint* key, uint keylength, uint* result);

	static __m256i check_bitmap(ulong* bitmap, uint bitmapSize, __m256i input);
	static __m256i lookup_cht(ulong* bitmap, uint bitmapSize, uint* chtpayload,
			uint chtsize, __m256i input);
};

#endif /* SRC_JOIN_CSTEP_CSTEPSIMD_H_ */
