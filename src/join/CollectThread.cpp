/*
 * CollectThread.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#include "CollectThread.h"
#include <memory.h>

#define NUM_THREAD 50

CollectThread::CollectThread(uint* base, uint from, uint to, Predicate* p) {
	this->base = base;
	this->from = from;
	this->to = to;
	this->p = p;

	this->_buffer = new uint[to - from];
	this->_bufferLength = 0;
}

CollectThread::~CollectThread() {
	delete[] this->_buffer;
}

uint* CollectThread::buffer() {
	return _buffer;
}

uint CollectThread::bufferLength() {
	return _bufferLength;
}

void CollectThread::run() {
	for (uint i = from; i < to; i++) {
		if (p->test(base[i]))
			this->_buffer[_bufferLength++] = base[i];
	}
}

uint CollectThread::collect(uint* src, uint* dest, uint length, Predicate* p) {

	CollectThread **threads = new CollectThread*[NUM_THREAD];
	uint step = length / NUM_THREAD;

	for (uint i = 0; i < NUM_THREAD; i++) {
		threads[i] = new CollectThread(src, i * step,
				i == NUM_THREAD - 1 ? length : (i + 1) * step, p);
		threads[i]->start();
	}
	for (uint i = 0; i < NUM_THREAD; i++) {
		threads[i]->wait();
	}

	uint counter = 0;
	for (uint i = 0; i < NUM_THREAD; i++) {
		memcpy(dest + counter, threads[i]->buffer(),
				threads[i]->bufferLength() * sizeof(uint));
		counter += threads[i]->bufferLength();
		delete threads[i];
	}
	delete[] threads;

	return counter;
}
