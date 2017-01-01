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
protected:
	__m256i alignas(32) buffer;
	int bufferSize;
public:
	static __m256i EMPTY;

	SimdBuffer();
	virtual ~SimdBuffer();

	void* operator new(size_t num);

	// Serve new data, and get compressed data (if any)
	__m256i serve(__m256i input, int* outputSize);
	// Read out all remaining data
	__m256i purge(int* outputSize);
protected:
	// Align the vector to left
	static __m256i align(__m256i input, int* size);
	// Shift 32-bit data to left 32-bit lane
	static __m256i shl(__m256i input, int offset);
	// Shift 32-bit data to right 32-bit lane
	static __m256i shr(__m256i input, int offset);
	// Merge two vectors
	static __m256i merge(__m256i a, __m256i b, int sizea);
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
