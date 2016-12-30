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
public:
	// Align the vector to left
	static __m256i align(__m256i input);
	// Shift 32-bit data to right 32-bit lane
	static __m256i shr(__m256i input, int offset);
protected:
	static __m256i FLAG_SHIFT;
	static __m256i FLAG_PERMUTE;

	static __m256i LOOKUP_SIZE;
	static __m256i LOOKUP_POS1;
	static __m256i LOOKUP_POS2;
	static __m256i LOOKUP_POS3;
	static __m256i LOOKUP_POS4;

	static __m256i PERMU_POS1;
	static __m256i PERMU_POS2;
	static __m256i PERMU_POS3;
	static __m256i PERMU_POS4;

	static __m256i SHL_POS[5];
	static __m256i SHR_POS[8];
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
