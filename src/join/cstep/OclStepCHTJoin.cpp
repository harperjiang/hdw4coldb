/*
 * OclStepCHTJoin.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#include "OclStepCHTJoin.h"
#include "../CounterThread.h"
#include "GatherThread.h"
#include "../../lookup/CHTConstants.h"
#include "../../lookup/CHT.h"
#include "../Predicate.h"

OclStepCHTJoin::OclStepCHTJoin(bool caf, bool cac, bool ep) :
		StepCHTJoin(caf, cac, ep) {

	env = new CLEnv(ep);
	_scanBitmap = new CLProgram(env, "scan_bitmap");
	_scanBitmap->fromFile("scan_bitmap.cl", 4);
	_scanCht = new CLProgram(env, "scan_chthash");
	_scanCht->fromFile("scan_chthash.cl", 6);
}

OclStepCHTJoin::~OclStepCHTJoin() {
	delete metaBuffer;
	delete bitmapBuffer;

	delete _scanBitmap;
	delete _scanCht;
	delete env;
}

void OclStepCHTJoin::init() {
	CHT* cht = (CHT*) _lookup;
	meta[0] = cht->bitmap_size * 32;
	meta[1] = cht->overflow->bucket_size;
	meta[2] = cht->payload_size;
	bitmapResultSize = _probeSize / RET_BITMAP_UNIT
			+ (_probeSize % RET_BITMAP_UNIT ? 1 : 0);
	bitmapResultSize += bitmapResultSize % 2 ? 0 : 1;
	meta[3] = bitmapResultSize;
	meta[4] = _probeSize;

	metaBuffer = new CLBuffer(env, meta, sizeof(uint) * 5,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);

	bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
}

void OclStepCHTJoin::filter() {

	CLBuffer* inputKeyBuffer = new CLBuffer(env, _probe,
			sizeof(uint) * _probeSize,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* bitmapResultBuffer = new CLBuffer(env, NULL,
			sizeof(uint64_t) * bitmapResultSize, CL_MEM_READ_WRITE);

	_scanBitmap->setBuffer(0, metaBuffer);
	_scanBitmap->setBuffer(1, bitmapBuffer);
	_scanBitmap->setBuffer(2, inputKeyBuffer);
	_scanBitmap->setBuffer(3, bitmapResultBuffer);

	_scanBitmap->execute(_probeSize);

	uint64_t* bitmapResult = (uint64_t*) bitmapResultBuffer->map(CL_MAP_READ);

	_timer.interval("filter_execute");

	// Gather
	chtInput = new uint[_probeSize];
	chtInputSize = GatherThread::gather(bitmapResult, bitmapResultSize, _probe,
			_probeSize, chtInput);
	_logger->debug("Records passed bitmap check: %u\n", chtInputSize);

	_timer.interval("filter_gather");

	bitmapResultBuffer->unmap();

	delete inputKeyBuffer;
	delete bitmapResultBuffer;

}

void OclStepCHTJoin::scanCht() {
	CHT* cht = (CHT*) _lookup;
	CLBuffer* passbitmapKeyBuffer = new CLBuffer(env, chtInput,
			chtInputSize * sizeof(uint),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht->keys,
			sizeof(uint) * cht->payload_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* hashpayloadBuffer = new CLBuffer(env, cht->overflow->buckets,
			sizeof(uint) * (meta[1] == 0 ? 1 : meta[1]),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* resultBuffer = new CLBuffer(env, NULL,
			chtInputSize * sizeof(uint),
			CL_MEM_WRITE_ONLY);

	_scanCht->setBuffer(0, metaBuffer);
	_scanCht->setBuffer(1, bitmapBuffer);
	_scanCht->setBuffer(2, chtpayloadBuffer);
	_scanCht->setBuffer(3, hashpayloadBuffer);
	_scanCht->setBuffer(4, passbitmapKeyBuffer);
	_scanCht->setBuffer(5, resultBuffer);

	_scanCht->execute(chtInputSize);

	uint* chtResult = (uint*) resultBuffer->map(CL_MAP_READ);

	_timer.interval("cht_execute");

	NotEqual nmax(0xffffffff);
	CounterThread::count(chtResult, chtInputSize, &nmax, _matched);

	_timer.interval("cht_count");
	resultBuffer->unmap();

	delete passbitmapKeyBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;
	delete resultBuffer;
}

void OclStepCHTJoin::scanHash() {
	// DO NOTHING
}

void OclStepCHTJoin::collect() {
	// DO NOTHING
}

