/*
 * CStepSimd.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_CSTEPSIMD_H_
#define SRC_JOIN_CSTEP_CSTEPSIMD_H_

#include "CStepOcl.h"

class CStepSimd: public CStep {
protected:
	uint* pattern;
	ulong* alignedBitmap;
	uint* alignedChtload;
private:
	__m256i hashFactor;
	__m256i ZERO;
	__m256i ONE;
	__m256i TWO;
	__m256i MAX;

	__m256i check_bitmap(__m256i input);
	__m256i lookup_cht(__m256i input);

public:
	CStepSimd();
	virtual ~CStepSimd();
protected:
	void buildProbe(kvlist* inner);

	void init();
	uint filter(uint* gathered);
	uint lookup(uint* key, uint keylength, uint* result);
};

#endif /* SRC_JOIN_CSTEP_CSTEPSIMD_H_ */
