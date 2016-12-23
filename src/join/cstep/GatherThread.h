/*
 * GatherThread.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_GATHERTHREAD_H_
#define SRC_JOIN_CSTEP_GATHERTHREAD_H_

#include "../../util/Thread.h"
#include "../../cstepjoin.h"

class GatherThread: public Thread {
private:
	uint64_t* bitmapResult;
	uint* innerkey;
	uint* dest;
	uint keystart;
	uint keystop;
	uint dstart;
public:
	GatherThread(uint64_t* bitmapResult, uint* innerkey, uint* dest,
			uint keystart, uint keystop, uint dstart);
	virtual ~GatherThread();
	void run();

	static void gather(uint* result);
};

#endif /* SRC_JOIN_CSTEP_GATHERTHREAD_H_ */
