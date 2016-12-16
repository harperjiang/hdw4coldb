/*
 * opencl_hash.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: harper
 */

#include "ocljoin.h"

void runHash(kvlist* outer, kvlist* inner, uint split, bool enableProfiling =
		false) {
	Logger* logger = Logger::getLogger("ocljoin-hash");

	logger->info("Running hash join\n");
	Hash* hash = new Hash();
	hash->build(outer->entries, outer->size);

	CLEnv* env = new CLEnv(enableProfiling);

	CLProgram* hashScan = new CLProgram(env, "scan_hash");
	hashScan->fromFile("scan_hash.cl", 4);

	uint32_t meta[2];
	meta[1] = hash->bucket_size;
	uint32_t* payload = new uint32_t[hash->bucket_size];
	for (uint32_t i = 0; i < hash->bucket_size; i++) {
		payload[i] = hash->buckets[i].key;
	}

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	Timer timer;
	timer.start();

	uint splitRound = 1;
	uint workSize = inner->size;
	if (split != 0) {
		splitRound = inner->size / split;
		workSize = split;
		while (workSize * splitRound < inner->size)
			splitRound++;
	}

	uint matched = 0;

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* payloadBuffer = new CLBuffer(env, payload,
			sizeof(uint32_t) * hash->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	for (uint sIndex = 0; sIndex < splitRound; sIndex++) {
		uint offset = sIndex * workSize;
		uint length =
				(sIndex + 1) * workSize > inner->size ?
						inner->size - sIndex * workSize : workSize;

		CLBuffer* innerKeyBuffer = new CLBuffer(env, innerkey + offset,
				sizeof(uint32_t) * length,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * length, CL_MEM_WRITE_ONLY);

		hashScan->setBuffer(0, metaBuffer);
		hashScan->setBuffer(1, payloadBuffer);
		hashScan->setBuffer(2, innerKeyBuffer);
		hashScan->setBuffer(3, resultBuffer);

		hashScan->execute(length);

		uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);
		for (uint32_t i = 0; i < length; i++) {
			matched += result[i] == 0xffffffff ? 0 : 1;
		}
		resultBuffer->unmap();

		delete innerKeyBuffer;
		delete resultBuffer;
	}

	timer.stop();
	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			matched);

	delete metaBuffer;
	delete payloadBuffer;

	delete hashScan;
	delete env;
	delete hash;
}
