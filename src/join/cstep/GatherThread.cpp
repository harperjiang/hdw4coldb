/*
 * GatherThread.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#include "GatherThread.h"

GatherThread::GatherThread(uint64_t* bitmapResult, uint* innerkey, uint* dest,
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
	for (uint32_t i = keystart; i < keystop; i++) {
		uint index = i / RET_BITMAP_UNIT;
		uint offset = i % RET_BITMAP_UNIT;
		if (bitmapResult[index] & ((ulong) 1) << offset) {
			dest[dindex++] = innerkey[i];
		}
	}
}

