/*
 * CStep.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStep.h"
#include "../../util/Timer.h"

CStep::CStep() {
	enableProfiling = false;
	_lookup = NULL;
	probe = NULL;
}

CStep::~CStep() {
	if (NULL != _lookup)
		delete _lookup;
	if (NULL != probe)
		delete probe;
}

void CStep::join(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling) {
	this->enableProfiling = enableProfiling;

	Timer timer;

	logger->info("Running CHT Step Join\n");

	buildLookup(outer);
	buildProbe(inner);

	uint workSize = inner->size;
	// OpenCL program compilation takes a long time
	this->init();

	timer.start();

	uint gatheredSize = workSize;
	uint* gatheredkey = new uint[gatheredSize];

	uint gatheredKeyLength = this->filter(gatheredkey);

	timer.pause();
	timer.resume();
	uint* result = new uint[gatheredKeyLength];
	uint resultSize = this->lookup(gatheredkey, gatheredKeyLength, result);

	timer.stop();
	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			resultSize);

	delete[] gatheredkey;
	delete[] result;
}

void CStep::buildLookup(kvlist* outer) {
	logger->info("Building Outer Table\n");
	_lookup = new CHT();
	_lookup->build(outer->entries, outer->size);
	logger->info("Building Outer Table Done\n");
}

void CStep::buildProbe(kvlist* inner) {
	this->probeSize = inner->size;
	this->probe = new uint[probeSize];
	for (uint i = 0; i < probeSize; i++) {
		probe[i] = inner->entries[i].key;
	}
}
