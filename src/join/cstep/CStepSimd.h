/*
 * CStepSimd.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_CSTEPSIMD_H_
#define SRC_JOIN_CSTEP_CSTEPSIMD_H_

#include "CStepOcl.h"

class CStepSimd: public CStepOco {
protected:
	uint* pattern;
public:
	CStepSimd();
	virtual ~CStepSimd();

	uint filter(uint* key, uint keylength, ulong* bitmap, uint bitmapSize,
			uint* gathered);
};

#endif /* SRC_JOIN_CSTEP_CSTEPSIMD_H_ */
