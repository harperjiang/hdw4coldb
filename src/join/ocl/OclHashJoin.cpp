/*
 * OclHashJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "OclHashJoin.h"
#include "../../util/Timer.h"
#include "../../lookup/CHT.h"
#include "../../util/Logger.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"
#include "../../lookup/LookupHelper.h"
#include "../CounterThread.h"

OclHashJoin::OclHashJoin() {
	_logger = Logger::getLogger("OclHashJoin");
}

OclHashJoin::~OclHashJoin() {
	// TODO Auto-generated destructor stub
}

void OclHashJoin::join(kvlist* outer, kvlist* inner, bool enableProfiling) {

	_logger->info("Running Hash Join with OpenCL\n");

	_logger->debug("Building Hash Table");
	Hash* hash = new Hash();
	hash->build(outer->entries, outer->size);
	_logger->debug("Hash Table Built");

	CLEnv* env = new CLEnv(enableProfiling);
	CLProgram* hashScan = new CLProgram(env, "scan_hash");
	hashScan->fromFile("scan_hash.cl", 4);

	uint32_t meta[2];
	meta[1] = hash->bucket_size;

	uint32_t* payload = hash->buckets;

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	_timer.start();

	uint workSize = inner->size;

	uint matched = 0;

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* payloadBuffer = new CLBuffer(env, payload,
			sizeof(uint32_t) * hash->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	uint offset = 0;
	uint length = workSize;

	CLBuffer* innerKeyBuffer = new CLBuffer(env, innerkey + offset,
			sizeof(uint32_t) * length,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* resultBuffer = new CLBuffer(env, NULL, sizeof(uint32_t) * length,
	CL_MEM_WRITE_ONLY);

	hashScan->setBuffer(0, metaBuffer);
	hashScan->setBuffer(1, payloadBuffer);
	hashScan->setBuffer(2, innerKeyBuffer);
	hashScan->setBuffer(3, resultBuffer);

	hashScan->execute(length);

	uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

	NotEqual nmax(0xffffffff);
	matched += CounterThread::count(result, length, &nmax);

	resultBuffer->unmap();

	delete innerKeyBuffer;
	delete resultBuffer;

	_timer.stop();
	_logger->info("Running time: %u ms, matched row %u\n", _timer.wallclockms(),
			matched);

	delete metaBuffer;
	delete payloadBuffer;

	delete hashScan;
	delete env;
	delete hash;
}
