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

OclCHTJoin::OclCHTJoin() {
	_logger = Logger::getLogger("OclCHTJoin");
}

OclCHTJoin::~OclCHTJoin() {
	// TODO Auto-generated destructor stub
}

void OclCHTJoin::join(kvlist* outer, kvlist* inner, bool enableProfiling) {
	_logger->info("Running CHT Join\n");
	_logger->info("Building Outer Table\n");
	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	_logger->info("Building Outer Table Done\n");

	uint32_t meta[3];
	meta[0] = cht->bitmap_size;
	meta[1] = cht->overflow->bucket_size;
	meta[2] = cht->payload_size;

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	uint32_t* cht_payload = cht->keys;
	uint32_t* hash_payload = cht->overflow->buckets;

	CLEnv* env = new CLEnv(enableProfiling);

	CLProgram* scanChtFull = new CLProgram(env, "scan_cht_full");
	scanChtFull->fromFile("scan_cht_full.cl", 6);

	uint workSize = inner->size;

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

	uint32_t matched = 0;
	uint offset = 0;
	uint length = workSize;

	CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey,
			sizeof(uint32_t) * length,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* resultBuffer = new CLBuffer(env, NULL, sizeof(uint32_t) * length,
	CL_MEM_READ_WRITE);
	CLBuffer* debugBuffer = new CLBuffer(env, NULL, sizeof(uint32_t) * length,
	CL_MEM_READ_WRITE);
	scanChtFull->setBuffer(0, metaBuffer);
	scanChtFull->setBuffer(1, bitmapBuffer);
	scanChtFull->setBuffer(2, chtpayloadBuffer);
	scanChtFull->setBuffer(3, hashpayloadBuffer);
	scanChtFull->setBuffer(4, innerkeyBuffer);
	scanChtFull->setBuffer(5, resultBuffer);

	scanChtFull->execute(length);

	uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

	//		for (uint32_t i = 0; i < length; i++) {
	//			if (result[i] != 0xffffffff)
	//				matched++;
	//		}
	matched += CounterThread::count(result, length, 50, 0xffffffff, false);

	resultBuffer->unmap();

	delete innerkeyBuffer;
	delete resultBuffer;
	delete debugBuffer;

	_timer.stop();

	_logger->info("Running time: %u ms, matched row %u\n", _timer.wallclockms(),
			matched);

	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;

	delete scanChtFull;
	delete env;

	delete cht;
}
