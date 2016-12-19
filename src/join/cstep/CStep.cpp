/*
 * CStep.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStep.h"
#include "../../util/Logger.h"
#include "../../util/Timer.h"

CStep::CStep() {
	enableProfiling = false;
}

CStep::~CStep() {

}

void CStep::join(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling) {
	this->enableProfiling = enableProfiling;

	Timer timer;
	Logger* logger = Logger::getLogger("CStep");

	logger->info("Running CHT Step Join\n");
	logger->info("Building Outer Table\n");

	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger->info("Building Outer Table Done\n");

	uint* innerkey = new uint[inner->size];
	for (uint i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	uint workSize = inner->size;
	// OpenCL program compilation takes a long time
	this->init(cht, innerkey, inner->size);

	timer.start();
	uint gatheredSize = workSize;
	uint* gatheredkey = new uint[gatheredSize];
	uint gatheredKeyLength = this->filter(innerkey, workSize, cht->bitmap,
			cht->bitmap_size, gatheredkey);
	uint result = this->lookup(cht, gatheredkey, gatheredKeyLength);

	timer.stop();
	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			result);

	delete[] innerkey;
	delete[] gatheredkey;
	delete cht;
}
