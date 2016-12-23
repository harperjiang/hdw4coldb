/*
 * LookupThread.h
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_SIMPLE_LOOKUPTHREAD_H_
#define SRC_JOIN_SIMPLE_LOOKUPTHREAD_H_

#include <sys/types.h>
#include "../Join.h"
#include "../../util/Thread.h"

class LookupThread: public Thread {
private:
	Lookup* lookup;
	uint* probe;
	uint _start;
	uint _stop;

	Matched* matched;
public:
	LookupThread(Lookup* lookup, uint* probe, uint start, uint stop);
	virtual ~LookupThread();

	void run();

	Matched* getMatched();
};

#endif /* SRC_JOIN_SIMPLE_LOOKUPTHREAD_H_ */
