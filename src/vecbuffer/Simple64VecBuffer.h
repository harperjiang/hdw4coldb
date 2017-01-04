/*
 * Simple64VecBuffer.h
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#ifndef SRC_VECBUFFER_SIMPLE64VECBUFFER_H_
#define SRC_VECBUFFER_SIMPLE64VECBUFFER_H_

#include <sys/types.h>
#include "VecBuffer.h"

class Simple64VecBuffer: public VecBuffer {

protected:
	ulong buffer[4];
	uint bufferSize = 0;

public:
	Simple64VecBuffer();
	virtual ~Simple64VecBuffer();

	__m256i serve(__m256i input, int *outputSize);
	__m256i purge(int *outputSize);
};

#endif /* SRC_VECBUFFER_SIMPLE64VECBUFFER_H_ */
