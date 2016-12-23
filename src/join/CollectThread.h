/*
 * CollectThread.h
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_COLLECTTHREAD_H_
#define SRC_JOIN_COLLECTTHREAD_H_

#include "Predicate.h"
#include "../util/Thread.h"

class CollectThread: public Thread {
private:
	uint* base;
	uint from;
	uint to;
	Predicate* p;

	uint* _buffer;
	uint _bufferLength;
public:
	CollectThread(uint* base, uint from, uint to, Predicate* p);
	virtual ~CollectThread();

	void run();

	uint* buffer();
	uint bufferLength();

	static uint collect(uint* from, uint* to, uint length, Predicate* p);
};

#endif /* SRC_JOIN_COLLECTTHREAD_H_ */
