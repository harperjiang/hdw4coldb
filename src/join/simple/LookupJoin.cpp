/*
 * LookupJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "LookupJoin.h"

LookupJoin::LookupJoin() {
	_logger = Logger::getLogger("LookupJoin");
	_matched = new Matched();
}

LookupJoin::~LookupJoin() {
	if (NULL != _lookup)
		delete _lookup;
	_lookup = NULL;
	delete _matched;
}

Matched* LookupJoin::getMatched() {
	return _matched;
}

void LookupJoin::join(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling) {

	_logger->info("Building lookup table\n");
	_lookup = buildLookup(outer);
	_logger->info("Building lookup table done\n");

	Matched* match = getMatched();
	_logger->info("Running %s join\n", _lookup->getName());

	_timer.start();

	for (uint32_t i = 0; i < inner->size; i++) {
		kv ikv = inner->entries[i];
		uint8_t* outerpl = _lookup->access(ikv.key);
		if (outerpl != NULL)
			match->match(ikv.key, outerpl, ikv.payload);
	}

	_timer.stop();

	_logger->info("Running time: %u, matched row %u\n", _timer.wallclockms(),
			match->getCounter());
	delete match;
}

