/*
 * LookupJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "LookupJoin.h"
#include "LookupThread.h"

LookupJoin::LookupJoin(uint numThread, bool ep) :
		Join(ep) {
	this->numThread = numThread;
}

LookupJoin::~LookupJoin() {

}

const char* LookupJoin::name() {
	return "LookupJoin";
}

void LookupJoin::join(kvlist* outer, kvlist* inner) {
	_logger->info("Building lookup table\n");
	buildLookup(outer);
	buildProbe(inner);
	_logger->info("Building lookup table done\n");

	_logger->info("Running %s join\n", _lookup->getName());

	_timer.start();

	if (numThread == 0) {
		joinSingleThread();
	} else {
		joinMultiThread();
	}

	_timer.stop();

	printSummary();
}

void LookupJoin::joinSingleThread() {
	Matched* match = getMatched();
	for (uint32_t i = 0; i < _probeSize; i++) {
		uint8_t* outerpl = _lookup->access(_probe[i]);
		if (outerpl != NULL) {
			match->match(_probe[i], outerpl, NULL);
		}
	}
}

void LookupJoin::joinMultiThread() {
	LookupThread** threads = new LookupThread*[numThread];

	uint step = _probeSize / numThread;
	for (uint i = 0; i < numThread; i++) {
		threads[i] = new LookupThread(_lookup, _probe, i * step,
				i == numThread - 1 ? _probeSize : (i + 1) * step);
		threads[i]->start();
	}

	for (uint i = 0; i < numThread; i++) {
		threads[i]->wait();
		_matched->merge(threads[i]->getMatched());
		delete threads[i];
	}

	delete[] threads;
}
