/*
 * CstepOco.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "CStepOcl.h"

#include "../../util/Thread.h"
#include "GatherThread.h"
#include "../../lookup/LookupHelper.h"

CStepOcl::CStepOcl() {
	cht_payload = NULL;
	hash_payload = NULL;
	env = NULL;
	scanBitmap = NULL;
	scanCht = NULL;
}

CStepOcl::~CStepOcl() {
	if (NULL != cht_payload)
		delete[] cht_payload;
	if (NULL != hash_payload)
		delete[] hash_payload;

	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;

	delete scanBitmap;
	delete scanCht;
	delete env;
}

void CStepOcl::init() {
	meta[0] = _lookup->bitmap_size * 32;
	meta[1] = _lookup->overflow->bucket_size;
	meta[2] = _lookup->payload_size;

	cht_payload = _lookup->keys;

	hash_payload = _lookup->overflow->buckets;

	env = new CLEnv(enableProfiling);

	scanBitmap = new CLProgram(env, "scan_bitmap");
	scanBitmap->fromFile("scan_bitmap.cl", 4);
	scanCht = new CLProgram(env, "scan_chthash");
	scanCht->fromFile("scan_chthash.cl", 6);

	bitmapBuffer = new CLBuffer(env, _lookup->bitmap,
			sizeof(uint64_t) * _lookup->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint) * _lookup->payload_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint) * (meta[1] == 0 ? 1 : meta[1]),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	bitmapResultSize = probeSize / RET_BITMAP_UNIT
			+ (probeSize % RET_BITMAP_UNIT ? 1 : 0);
	bitmapResultSize += bitmapResultSize % 2 ? 0 : 1;
	meta[3] = bitmapResultSize;
	meta[4] = probeSize;

	metaBuffer = new CLBuffer(env, meta, sizeof(uint) * 5,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);

}

uint CStepOcl::filter(uint* gathered) {
	CLBuffer* inputKeyBuffer = new CLBuffer(env, probe,
			sizeof(uint) * probeSize,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* bitmapResultBuffer = new CLBuffer(env, NULL,
			sizeof(uint64_t) * bitmapResultSize, CL_MEM_READ_WRITE);

	scanBitmap->setBuffer(0, metaBuffer);
	scanBitmap->setBuffer(1, bitmapBuffer);
	scanBitmap->setBuffer(2, inputKeyBuffer);
	scanBitmap->setBuffer(3, bitmapResultBuffer);

	scanBitmap->execute(probeSize);

	uint64_t* bitmapResult = (uint64_t*) bitmapResultBuffer->map(CL_MAP_READ);

	// Gather
	uint numPassBitmap = gather(bitmapResult, gathered);

	bitmapResultBuffer->unmap();

	delete inputKeyBuffer;
	delete bitmapResultBuffer;

	return numPassBitmap;
}

uint CStepOcl::lookup(uint* key, uint keylength, uint* result) {
	CLBuffer* passbitmapKeyBuffer = new CLBuffer(env, key,
			keylength * sizeof(uint),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* resultBuffer = new CLBuffer(env, NULL, keylength * sizeof(uint),
	CL_MEM_WRITE_ONLY);

	scanCht->setBuffer(0, metaBuffer);
	scanCht->setBuffer(1, bitmapBuffer);
	scanCht->setBuffer(2, chtpayloadBuffer);
	scanCht->setBuffer(3, hashpayloadBuffer);
	scanCht->setBuffer(4, passbitmapKeyBuffer);
	scanCht->setBuffer(5, resultBuffer);

	scanCht->execute(keylength);

	uint* chtResult = (uint*) resultBuffer->map(CL_MAP_READ);

	uint matched = 0;
	for (uint i = 0; i < keylength; i++) {
		if (chtResult[i] != 0xffffffff) {
			matched++;
		}
	}
	resultBuffer->unmap();

	delete passbitmapKeyBuffer;
	delete resultBuffer;

	return matched;
}

uint CStepOcl::gather(ulong* bitmap, uint* gathered) {
	uint bitmapSize = bitmapResultSize;
	uint workSize = probeSize;

	uint threadNum = 30;
	Thread** gatherThreads = new Thread*[threadNum];
	uint destStart[threadNum];

	uint threadAlloc[threadNum];
	::memset(threadAlloc, 0, sizeof(uint) * threadNum);

	uint threadBitmapSize = bitmapSize / threadNum;

	for (uint i = 0; i < threadBitmapSize; i++) {
		for (uint t = 0; t < threadNum; t++) {
			threadAlloc[t] += popcount64(bitmap[t * threadBitmapSize + i]);
		}
	}
	for (uint i = 0; i < bitmapSize % threadNum; i++) {
		threadAlloc[threadNum - 1] += popcount64(
				bitmap[threadBitmapSize * threadNum + i]);
	}

	uint sum = 0;
	for (uint i = 0; i < threadNum; i++) {
		destStart[i] = sum;
		sum += threadAlloc[i];
	}

	uint keyPerThread = threadBitmapSize * RET_BITMAP_UNIT;

	for (uint i = 0; i < threadNum; i++) {
		uint keyStart = i * keyPerThread;
		uint keyEnd = i == threadNum - 1 ? workSize : keyPerThread * (i + 1);
		gatherThreads[i] = new GatherThread(bitmap, probe, gathered, keyStart,
				keyEnd, destStart[i]);
		gatherThreads[i]->start();
	}
	for (uint i = 0; i < threadNum; i++) {
		gatherThreads[i]->wait();
	}

	delete[] gatherThreads;
	return sum;
}
