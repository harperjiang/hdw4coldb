/*
 * OclCHTJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "OclCHTJoin.h"

#include "../../util/Timer.h"
#include "../../lookup/CHT.h"
#include "../../util/Logger.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"
#include "../../lookup/LookupHelper.h"
#include "../CounterThread.h"

OclCHTJoin::OclCHTJoin(bool ep) :
		Join(ep) {
	_logger = Logger::getLogger("OclCHTJoin");
	env = new CLEnv(enableProfiling);
	scanChtFull = new CLProgram(env, "scan_cht_full");
	scanChtFull->fromFile("scan_cht_full.cl", 6);
}

OclCHTJoin::~OclCHTJoin() {
	delete scanChtFull;
	delete env;
}

Lookup* OclCHTJoin::createLookup() {
	return new CHT();
}

void OclCHTJoin::join(kvlist* outer, kvlist* inner) {
	_logger->info("Running CHT Join with OpenCL\n");

	buildLookup(outer);
	buildProbe(inner);

	CHT* cht = (CHT*) _lookup;
	uint32_t meta[3];
	meta[0] = cht->bitmap_size;
	meta[1] = cht->overflow->bucket_size;
	meta[2] = cht->payload_size;

	uint32_t* cht_payload = cht->keys;
	uint32_t* hash_payload = cht->overflow->buckets;

	_timer.start();

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 3,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);

	CLBuffer* bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint32_t) * cht->payload_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint32_t)
					* ((0 == cht->overflow->bucket_size) ?
							1 : cht->overflow->bucket_size),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* innerkeyBuffer = new CLBuffer(env, _probe,
			sizeof(uint32_t) * _probeSize,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* resultBuffer = new CLBuffer(env, NULL,
			sizeof(uint32_t) * _probeSize,
			CL_MEM_READ_WRITE);

	scanChtFull->setBuffer(0, metaBuffer);
	scanChtFull->setBuffer(1, bitmapBuffer);
	scanChtFull->setBuffer(2, chtpayloadBuffer);
	scanChtFull->setBuffer(3, hashpayloadBuffer);
	scanChtFull->setBuffer(4, innerkeyBuffer);
	scanChtFull->setBuffer(5, resultBuffer);

	scanChtFull->execute(_probeSize);

	_timer.interval("cht_scan");

	uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

	NotEqual nmax(0xffffffff);
	CounterThread::count(result, _probeSize, &nmax, _matched);

	resultBuffer->unmap();

	_timer.interval("collect_result");

	_timer.stop();
	printSummary();

	delete innerkeyBuffer;
	delete resultBuffer;
	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;
}
