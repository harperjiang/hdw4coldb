/*
 * Join.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "Join.h"
#include "../util/Logger.h"

Join::Join(bool enableProfiling) {
	_logger = NULL;
	_matched = new Matched();
	this->enableProfiling = enableProfiling;
}

Join::~Join() {
	if (NULL != _lookup)
		delete _lookup;
	_lookup = NULL;
	if ( NULL != _probe) {
		delete[] _probe;
	}
	_probe = NULL;
}

void Join::buildLookup(kvlist* outer) {
	_lookup = createLookup();
	if (NULL != _lookup)
		_lookup->build(outer->entries, outer->size);
}

void Join::buildProbe(kvlist* inner) {
	_probe = new uint32_t[inner->size];
	for (uint i = 0; i < inner->size; i++) {
		_probe[i] = inner->entries[i].key;
	}
	_probeSize = inner->size;
}

void Join::printSummary() {
	_logger->info("Running time: %ums, matched row %u\n", _timer.wallclockms(),
			getMatched()->getCounter());
	for (uint i = 0; i < _timer.numInterval(); i++) {
		_logger->info("Phase %s: %u ms\n", _timer.name(i), _timer.interval(i));
	}
}

void Join::setMatched(Matched* m) {
	this->_matched = m;
}

Matched* Join::getMatched() {
	return this->_matched;
}
