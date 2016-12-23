/*
 * CounterThread.cpp
 *
 *  Created on: Dec 20, 2016
 *      Author: harper
 */

#include "CounterThread.h"

#define NUM_THREAD 50

CounterThread::CounterThread(uint* base, uint begin, uint end, Predicate* p) {
	this->base = base;
	this->begin = begin;
	this->end = end;
	this->p = p;
	this->counter = 0;
}

CounterThread::~CounterThread() {
}

void CounterThread::run() {
	for (uint i = begin; i < end; i++) {
		counter += p->test(base[i]);
	}
}

uint CounterThread::getCounter() {
	return this->counter;
}

uint CounterThread::count(uint* base, uint length, Predicate* p) {
	CounterThread **threads = new CounterThread*[NUM_THREAD];
	uint step = length / NUM_THREAD;

	for (uint i = 0; i < NUM_THREAD; i++) {
		threads[i] = new CounterThread(base, i * step,
				i == NUM_THREAD - 1 ? length : (i + 1) * step, p);
		threads[i]->start();
	}
	for (uint i = 0; i < NUM_THREAD; i++) {
		threads[i]->wait();
	}

	uint counter = 0;
	for (uint i = 0; i < NUM_THREAD; i++) {
		counter += threads[i]->getCounter();
		delete threads[i];
	}
	delete[] threads;

	return counter;
}
