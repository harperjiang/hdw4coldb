/*
 * SimdBuffer.h
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_SIMD_SIMDBUFFER_H_
#define SRC_JOIN_SIMD_SIMDBUFFER_H_

#include <immintrin.h>

class SimdBuffer {
public:
	SimdBuffer();
	virtual ~SimdBuffer();

	static __m256i EMPTY;
	__m256i serve(__m256i input);
	__m256i purge();
protected:
	int numZero(__m256i input);
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
