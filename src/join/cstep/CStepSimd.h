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
#include "../../simd/SimdHelper.h"

class CStepSimd: public CStep {
	friend class CheckBitmapTransform;
	friend class LookupChtTransform;
protected:
	ulong* alignedBitmap = NULL;
	uint* alignedChtload = NULL;

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

class CheckBitmapTransform: public SimdTransform {
private:
	CStepSimd* owner;
public:
	CheckBitmapTransform(CStepSimd* css) {
		owner = css;
	}
	virtual ~CheckBitmapTransform() {
	}

	__m256i transform(__m256i);
};

class LookupChtTransform: public SimdTransform {
private:
	CStepSimd* owner;
public:
	LookupChtTransform(CStepSimd* css) {
		owner = css;
	}
	virtual ~LookupChtTransform() {
	}
	__m256i transform(__m256i);
};

#endif /* SRC_JOIN_CSTEP_CSTEPSIMD_H_ */
