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
	// Serve new data, and get compressed data (if any)
	__m256i serve(__m256i input);
	// Read out all remaining data
	__m256i purge();
protected:

public:
	// Shift 32-bit data to left 32-bit lane
	static __m256i shiftLeft(__m256i input, int offset);
	// Shift 32-bit data to right 32-bit lane
	static __m256i shiftRight(__m256i input, int offset);
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
