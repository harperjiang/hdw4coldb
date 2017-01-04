/*
 * SimdBuffer.h
 *
 *  Created on: Dec 28, 2016
 *      Author: harper
 */

#ifndef SRC_VECBUFFER_SIMDBUFFER_H_
#define SRC_VECBUFFER_SIMDBUFFER_H_

#include "VecBuffer.h"

class SimdVecBuffer: public VecBuffer {
protected:
	// Buffer for primary input
	__m256i buffer;
	int bufferSize;
public:
	static __m256i EMPTY;

	void* operator new(size_t num);

	SimdVecBuffer();
	virtual ~SimdVecBuffer();

	// Serve new data, and get compressed data (if any)
	__m256i serve(__m256i input, int* outputSize);
	// Read out all remaining data
	__m256i purge(int* outputSize);
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

#endif /* SRC_VECBUFFER_SIMDBUFFER_H_ */
