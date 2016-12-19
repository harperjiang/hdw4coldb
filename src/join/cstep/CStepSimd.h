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

private:
	__m256i hashFactor;

	__m256i check_bitmap(__m256i input);

public:
	CStepSimd();
	virtual ~CStepSimd();

	void buildProbe(kvlist* inner);

	void init();
	uint filter(uint* gathered);
	uint lookup(uint* key, uint keylength);
};

#endif /* SRC_JOIN_CSTEP_CSTEPSIMD_H_ */
