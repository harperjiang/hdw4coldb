/*
 * SimpleVecBuffer.h
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#ifndef SRC_VECBUFFER_SIMPLEVECBUFFER_H_
#define SRC_VECBUFFER_SIMPLEVECBUFFER_H_

#include "VecBuffer.h"

class SimpleVecBuffer: public VecBuffer {
protected:
	int buffer[8];
	int bufferSize;

public:
	SimpleVecBuffer();
	virtual ~SimpleVecBuffer();

	__m256i serve(__m256i input, int *outputSize);
	__m256i purge(int *outputSize);
};

#endif /* SRC_VECBUFFER_SIMPLEVECBUFFER_H_ */
