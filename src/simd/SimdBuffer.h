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
	static __m256i align(__m256i input, int* size);
	// Shift 32-bit data to right 32-bit lane
	static __m256i shr(__m256i input, int offset);
	// Merge two vectors
	static __m256i merge(__m256i a, __m256i b, int sizea);
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
