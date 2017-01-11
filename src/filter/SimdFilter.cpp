/*
 * SimdFilter.cpp
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#include "SimdFilter.h"

#include "../vecbuffer/SimdVecBuffer.h"

SimdFilter::SimdFilter(OP op, number target) :
		Filter() {
	this->pred = SimdPred::create(op, target);
}

SimdFilter::~SimdFilter() {
	delete this->pred;
}

void SimdFilter::filter(uint* input, uint inputSize, uint* output) {
	SimdVecBuffer* vecbuf = new SimdVecBuffer();

	uint round = inputSize / 8;
	int ressize;
	uint outputLoc = 0;
	for (uint i = 0; i < round; i++) {
		__m256i loaded = _mm256_stream_load_si256((__m256i *) (input + 8 * i));
		__m256i filtered = pred->testInteger(loaded);
		__m256i buffered = vecbuf->serve(filtered, &ressize);
		if (ressize) {
			_mm256_stream_si256((__m256i *) (output + (outputLoc++) * 8),
					buffered);
		}
	}
	delete vecbuf;
}
