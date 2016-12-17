/*
 * chtstep.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: harper
 */

#include "ocljoin.h"

class GatherThread {

public:
	GatherThread(uint* innerkey, uint* bitmapResult, uint start, uint stop) {

	}

	void run() {

	}
};

void gather(uint* innerkey, uint* bitmapResult, uint bitmapSize,
		uint* passedkey, uint workSize, uint* counter, Timer* timer) {
	*counter = 0;
	for (uint32_t i = 0; i < workSize; i++) {
		uint index = i / BITMAP_UNIT;
		uint offset = i % BITMAP_UNIT;
		if (bitmapResult[index] & 1 << offset) {
			passedkey[*counter++] = innerkey[i];
		}
	}

	timer->pause();
	timer->resume();

	uint sum = 0;
	for (uint i = 0; i < bitmapSize; i++) {
		sum += popcount(bitmapResult[i]);
	}
	timer->pause();
	timer->resume();
	sum = 0;
	for (uint i = 0; i < bitmapSize / 2; i++) {
		sum += popcount64(((uint64_t*) bitmapResult)[i]);
	}
}

void runChtStep(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling) {
	Timer timer;
	Timer timer2;
	Logger* logger = Logger::getLogger("ocljoin-chtstep");

	logger->info("Running CHT Step Join\n");
	logger->info("Building Outer Table\n");

	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger->info("Building Outer Table Done\n");

	uint32_t meta[5];

	meta[0] = cht->bitmap_size * 32;
	meta[1] = cht->overflow->bucket_size;
	meta[2] = cht->payload_size;

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	uint32_t* cht_payload = new uint32_t[cht->payload_size];
	for (uint32_t i = 0; i < cht->payload_size; i++) {
		cht_payload[i] = cht->payloads[i].key;
	}

	uint32_t* hash_payload = new uint32_t[cht->overflow->bucket_size];
	for (uint32_t i = 0; i < cht->overflow->bucket_size; i++) {
		hash_payload[i] = cht->overflow->buckets[i].key;
	}

	uint workSize = inner->size;
	uint32_t* passedkey = new uint32_t[workSize];

	CLEnv* env = new CLEnv(enableProfiling);

	CLProgram* scanBitmap = new CLProgram(env, "scan_bitmap");
	scanBitmap->fromFile("scan_bitmap.cl", 4);
	CLProgram* scanCht = new CLProgram(env, "scan_chthash");
	scanCht->fromFile("scan_chthash.cl", 6);

	uint matched = 0;

	uint bitmapResultSize = workSize / BITMAP_UNIT
			+ (workSize % BITMAP_UNIT ? 1 : 0);

	meta[3] = bitmapResultSize;
	meta[4] = workSize;

	timer.start();

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 5,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
	CLBuffer* bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint32_t) * cht->payload_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint32_t) * (meta[1] == 0 ? 1 : meta[1]),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey,
			sizeof(uint32_t) * workSize,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* bitmapResultBuffer = new CLBuffer(env, NULL,
			sizeof(uint) * bitmapResultSize, CL_MEM_READ_WRITE);

	scanBitmap->setBuffer(0, metaBuffer);
	scanBitmap->setBuffer(1, bitmapBuffer);
	scanBitmap->setBuffer(2, innerkeyBuffer);
	scanBitmap->setBuffer(3, bitmapResultBuffer);

	scanBitmap->execute(workSize);

	uint* bitmapResult = (uint*) bitmapResultBuffer->map(CL_MAP_READ);

// Gather
	timer.pause();
	timer2.start();
	uint32_t counter = 0;

	gather(innerkey, bitmapResult, bitmapResultSize, passedkey, workSize,
			&counter, &timer);

	bitmapResultBuffer->unmap();

	uint numPassBitmap = counter;

	timer2.stop();
	timer.resume();

	delete innerkeyBuffer;
	delete bitmapResultBuffer;

	CLBuffer* passbitmapKeyBuffer = new CLBuffer(env, passedkey,
			numPassBitmap * sizeof(uint32_t),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* resultBuffer = new CLBuffer(env, NULL,
			numPassBitmap * sizeof(uint32_t), CL_MEM_WRITE_ONLY);

	scanCht->setBuffer(0, metaBuffer);
	scanCht->setBuffer(1, bitmapBuffer);
	scanCht->setBuffer(2, chtpayloadBuffer);
	scanCht->setBuffer(3, hashpayloadBuffer);
	scanCht->setBuffer(4, passbitmapKeyBuffer);
	scanCht->setBuffer(5, resultBuffer);

	scanCht->execute(numPassBitmap);

	uint32_t* chtResult = (uint32_t*) resultBuffer->map(CL_MAP_READ);

	for (uint32_t i = 0; i < numPassBitmap; i++) {
		if (chtResult[i] != 0xffffffff) {
			matched++;
		}
	}
	resultBuffer->unmap();

	delete passbitmapKeyBuffer;
	delete resultBuffer;

	timer.stop();

	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			matched);
	for (uint i = 0; i < timer2.numInterval(); i++) {
		logger->info("Gather time %d: %u ms\n", i, timer2.interval(i));
	}
	delete[] passedkey;
	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;

	delete scanBitmap;
	delete scanCht;
	delete env;

	delete cht;
}

