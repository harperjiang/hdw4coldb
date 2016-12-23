/*
 * GatherThread.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "GatherThread.h"
#include "../../lookup/CHTConstants.h"
#include "../../lookup/LookupHelper.h"

#define NUM_THREAD 50

GatherThread::GatherThread(ulong* bitmapResult, uint* innerkey, uint* dest,
		uint keystart, uint keystop, uint dstart) {
	this->bitmapResult = bitmapResult;
	this->innerkey = innerkey;
	this->dest = dest;
	this->keystart = keystart;
	this->keystop = keystop;
	this->dstart = dstart;
}

GatherThread::~GatherThread() {

}

void GatherThread::run() {
	uint dindex = dstart;
	for (uint i = keystart; i < keystop; i++) {
		uint index = i / RET_BITMAP_UNIT;
		uint offset = i % RET_BITMAP_UNIT;
		if (bitmapResult[index] & ((ulong) 1) << offset) {
			dest[dindex++] = innerkey[i];
		}
	}
}

uint GatherThread::gather(ulong* bitmap, uint bitmapSize, uint* probe,
		uint probeSize, uint* gathered) {

	uint workSize = probeSize;

	Thread** gatherThreads = new Thread*[NUM_THREAD];
	uint destStart[NUM_THREAD];

	uint threadAlloc[NUM_THREAD];
	::memset(threadAlloc, 0, sizeof(uint) * NUM_THREAD);

	uint threadBitmapSize = bitmapSize / NUM_THREAD;

	for (uint i = 0; i < threadBitmapSize; i++) {
		for (uint t = 0; t < NUM_THREAD; t++) {
			threadAlloc[t] += popcount64(bitmap[t * threadBitmapSize + i]);
		}
	}
	for (uint i = 0; i < bitmapSize % NUM_THREAD; i++) {
		threadAlloc[NUM_THREAD - 1] += popcount64(
				bitmap[threadBitmapSize * NUM_THREAD + i]);
	}

	uint sum = 0;
	for (uint i = 0; i < NUM_THREAD; i++) {
		destStart[i] = sum;
		sum += threadAlloc[i];
	}

	uint keyPerThread = threadBitmapSize * RET_BITMAP_UNIT;

	for (uint i = 0; i < NUM_THREAD; i++) {
		uint keyStart = i * keyPerThread;
		uint keyEnd = i == NUM_THREAD - 1 ? workSize : keyPerThread * (i + 1);
		gatherThreads[i] = new GatherThread(bitmap, probe, gathered, keyStart,
				keyEnd, destStart[i]);
		gatherThreads[i]->start();
	}
	for (uint i = 0; i < NUM_THREAD; i++) {
		gatherThreads[i]->wait();
		delete gatherThreads[i];
	}

	delete[] gatherThreads;
	return sum;
}
