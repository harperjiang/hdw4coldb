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
	probeSize = 0;

	_logger = Logger::getLogger("CStep");
}

CStep::~CStep() {
	if (NULL != _lookup)
		delete _lookup;
	if (NULL != probe)
		delete probe;
}

void CStep::join(kvlist* outer, kvlist* inner, bool enableProfiling) {
	this->enableProfiling = enableProfiling;


	_logger->info("Running CHT Step Join\n");

	buildLookup(outer);
	buildProbe(inner);

	uint workSize = inner->size;
	// Initialization may take a long time
	this->init();

	_timer.start();

	uint gatheredSize = workSize;
	uint* gatheredkey = new uint[gatheredSize];
	uint gatheredKeyLength = this->filter(gatheredkey);
	_timer.interval("filter");

	uint* result = new uint[gatheredKeyLength];
	uint resultSize = this->lookup(gatheredkey, gatheredKeyLength, result);
	_timer.interval("lookup");
	_timer.stop();

	_logger->info("Running time: %u ms, matched row %u\n", _timer.wallclockms(),
			resultSize);
	for (int i = 0; i < _timer.numInterval(); i++) {
		_logger->info("Phase %s: %u ms\n", _timer.name(i), _timer.interval(i));
	}
	delete[] gatheredkey;
	delete[] result;
}

void CStep::buildLookup(kvlist* outer) {
	_logger->info("Building Outer Table\n");
	_lookup = new CHT();
	_lookup->build(outer->entries, outer->size);
	_logger->info("Building Outer Table Done\n");
}

void CStep::buildProbe(kvlist* inner) {
	this->probeSize = inner->size;
	this->probe = new uint[probeSize];
	for (uint i = 0; i < probeSize; i++) {
		probe[i] = inner->entries[i].key;
	}
}
