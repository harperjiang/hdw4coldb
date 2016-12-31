/*
 * SimdBuffer64.h
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#ifndef SRC_SIMD_SIMDBUFFER64_H_
#define SRC_SIMD_SIMDBUFFER64_H_

#include <immintrin.h>
#include <sys/types.h>

class SimdBuffer64 {
protected:
	static __m256i EMPTY;
	__m256i buffer;
	uint bufferSize;
public:
	SimdBuffer64();
	virtual ~SimdBuffer64();

	// Serve new data, and get compressed data (if any)
	__m256i serve(__m256i input, int* outputSize);
	// Read out all remaining data
	__m256i purge(int* outputSize);
protected:
	__m256i align(__m256i input, int *inputSize);
	__m256i merge(__m256i a, __m256i b, int asize);
	__m256i shl(__m256i a, int offset);
	__m256i shr(__m256i a, int offset);
};

#endif /* SRC_SIMD_SIMDBUFFER64_H_ */
