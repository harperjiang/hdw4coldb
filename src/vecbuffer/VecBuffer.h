/*
 * VecBuffer.h
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#ifndef SRC_VECBUFFER_VECBUFFER_H_
#define SRC_VECBUFFER_VECBUFFER_H_

#include <immintrin.h>

class VecBuffer {
public:
	VecBuffer();
	virtual ~VecBuffer();

	virtual __m256i serve(__m256i input, int* outputSize);
	virtual __m256i purge(int* outputSize);
};

#endif /* SRC_VECBUFFER_VECBUFFER_H_ */
