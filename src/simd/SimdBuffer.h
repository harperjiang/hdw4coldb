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
	// Buffer for primary input
	__m256i buffer;
	// Buffer for secondary input
	__m256i extraData;
	__m256i extraBuffer;
	int bufferSize;
public:
	static __m256i EMPTY;

	void* operator new(size_t num);

	SimdBuffer();
	virtual ~SimdBuffer();

	// Serve new data, and get compressed data (if any)
	__m256i serve(__m256i input, __m256i extra, int* outputSize);
	// Read out all remaining data
	__m256i purge(int* outputSize);
	__m256i getExtra();
protected:
	/*
	 * Align the vector to left
	 */
	static __m256i align(__m256i input, int* size, __m256i* pattern);
	/**
	 * Shift left
	 */
	static __m256i shl(__m256i input, int offset);
	/**
	 * Shift right
	 */
	static __m256i shr(__m256i input, int offset);
	/**
	 * Merge two vectors
	 */
	static __m256i merge(__m256i a, __m256i b, int sizea);
};

#endif /* SRC_JOIN_SIMD_SIMDBUFFER_H_ */
