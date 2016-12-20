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

class CounterThread: public Thread {
public:
	CounterThread(uint* base, uint begin, uint end, uint value, bool equal);
	virtual ~CounterThread();

	void run();

	static uint count(uint* base, uint length, uint numThread, uint value,
			bool equal);
};

#endif /* SRC_JOIN_COUNTERTHREAD_H_ */
