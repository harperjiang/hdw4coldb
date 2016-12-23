/*
 * CounterThread.h
 *
 * Use multi-thread to count the number of 0's in a list
 *
 *  Created on: Dec 20, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_COUNTERTHREAD_H_
#define SRC_JOIN_COUNTERTHREAD_H_

#include <stdlib.h>
#include "../util/Thread.h"
#include "Predicate.h"

class CounterThread: public Thread {
private:
	uint* base;
	uint begin;
	uint end;
	Predicate* p;

	uint counter;
public:
	CounterThread(uint* base, uint begin, uint end, Predicate* p);
	virtual ~CounterThread();

	void run();

	uint getCounter();

	static uint count(uint* base, uint length, Predicate* p);
};

#endif /* SRC_JOIN_COUNTERTHREAD_H_ */
