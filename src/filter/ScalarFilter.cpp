/*
 * ScalarFilter.cpp
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#include "ScalarFilter.h"

ScalarFilter::ScalarFilter(OP op, number target) :
		Filter() {
	this->pred = ScalarPred::create(op, target);
}

ScalarFilter::~ScalarFilter() {
	delete this->pred;
}

void ScalarFilter::filter(uint* input, uint inputSize, uint* output,
		uint* outputSize) {
	uint counter = 0;
	for (uint i = 0; i < inputSize; i++) {
		if (pred->testInteger(input[i])) {
			output[counter] = input[i];
		}
	}
	*outputSize = counter;
}
