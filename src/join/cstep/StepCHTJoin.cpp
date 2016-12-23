/*
 * StepCHTJoin.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#include "StepCHTJoin.h"

#include "../../lookup/CHT.h"

StepCHTJoin::StepCHTJoin(bool collectAfterFilter, bool collectAfterCht, bool ep) :
		Join(ep) {
	this->collectAfterFilter = collectAfterFilter;
	this->collectAfterCht = collectAfterCht;
}

StepCHTJoin::~StepCHTJoin() {
	if (NULL != chtInput)
		delete[] chtInput;
	if (NULL != chtResult)
		delete[] chtResult;
	if (NULL != hashInput)
		delete[] hashInput;
	if (NULL != hashResult)
		delete[] hashResult;
}

Lookup* StepCHTJoin::createLookup() {
	return new CHT();
}

void StepCHTJoin::join(kvlist* outer, kvlist* inner) {
	_logger->info("Running CHT Join in steps\n");

	buildLookup(outer);
	buildProbe(inner);

	_timer.start();
	init();
	_timer.interval("init");
	filter();
	_timer.interval("filter");
	scanCht();
	_timer.interval("scan_cht");
	scanHash();
	_timer.interval("scan_hash");
	collect();
	_timer.interval("collect_result");

	printSummary();
}
