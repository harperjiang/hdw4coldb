/*
 * LookupThread.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#include "LookupThread.h"

LookupThread::LookupThread(Lookup* lookup, uint* probe, uint start, uint stop) {
	this->lookup = lookup;
	this->probe = probe;
	this->_start = start;
	this->_stop = stop;

	this->matched = new Matched();
}

LookupThread::~LookupThread() {
	delete this->matched;
}

void LookupThread::run() {
	for (uint i = _start; i < _stop; i++) {
		uint8_t* outer = lookup->access(probe[i]);
		if (NULL != outer) {
			matched->match(probe[i], NULL, outer);
		}
	}
}

Matched* LookupThread::getMatched() {
	return matched;
}
