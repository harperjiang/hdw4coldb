/*
 * OclHashJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "OclHashJoin.h"

#include <CL/cl.h>
#include <stddef.h>
#include <sys/types.h>
#include <cstdint>

#include "../../lookup/Hash.h"
#include "../../lookup/Lookup.h"
#include "../../opencl/CLBuffer.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"
#include "../../util/Logger.h"
#include "../CounterThread.h"
#include "../Predicate.h"

OclHashJoin::OclHashJoin(bool ep) :
		Join(ep) {
	if (NULL == _logger)
		_logger = Logger::getLogger("OclHashJoin");

	env = new CLEnv(ep);
	hashScan = new CLProgram(env, "scan_hash");
	hashScan->fromFile("scan_hash.cl", 4);
}

OclHashJoin::~OclHashJoin() {
	delete hashScan;
	delete env;
}

Lookup* OclHashJoin::createLookup() {
	return new Hash();
}

void OclHashJoin::join(kvlist* outer, kvlist* inner) {

	_logger->info("Running Hash Join with OpenCL\n");

	buildLookup(outer);
	buildProbe(inner);

	Hash* hash = (Hash*) _lookup;

	uint32_t meta[2];
	meta[1] = hash->bucket_size;

	uint32_t* payload = hash->buckets;

	_timer.start();

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* payloadBuffer = new CLBuffer(env, payload,
			sizeof(uint32_t) * hash->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* innerKeyBuffer = new CLBuffer(env, _probe,
			sizeof(uint32_t) * _probeSize,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* resultBuffer = new CLBuffer(env, NULL,
			sizeof(uint32_t) * _probeSize,
			CL_MEM_WRITE_ONLY);

	hashScan->setBuffer(0, metaBuffer);
	hashScan->setBuffer(1, payloadBuffer);
	hashScan->setBuffer(2, innerKeyBuffer);
	hashScan->setBuffer(3, resultBuffer);

	hashScan->execute(_probeSize);

	_timer.interval("hash_scan");

	uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);
	NotEqual nmax(0xffffffff);
	CounterThread::count(result, _probeSize, &nmax, _matched);
	resultBuffer->unmap();

	_timer.interval("collect_result");

	_timer.stop();
	printSummary();

	delete innerKeyBuffer;
	delete resultBuffer;
	delete metaBuffer;
	delete payloadBuffer;
}
