/*
 * VecBuffer.cpp
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#include "VecBuffer.h"

VecBuffer::VecBuffer() {
	// TODO Auto-generated constructor stub

}

VecBuffer::~VecBuffer() {
	// TODO Auto-generated destructor stub
}

__m256i VecBuffer::serve(__m256i input, int* outputSize) {
	*outputSize = 8;
	return input;
}

__m256i VecBuffer::purge(int* outputSize) {
	*outputSize = 0;
	return _mm256_setzero_si256();
}
