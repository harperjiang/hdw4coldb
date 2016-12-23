/*
 * GatherThread.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_GATHERTHREAD_H_
#define SRC_JOIN_CSTEP_GATHERTHREAD_H_

#include <sys/types.h>
#include <stdlib.h>

#include "../../lookup/CHTConstants.h"
#include "../../util/Thread.h"

class GatherThread: public Thread {
private:
	ulong* bitmapResult;
	uint* innerkey;
	uint* dest;
	uint keystart;
	uint keystop;
	uint dstart;
public:
	GatherThread(ulong* bitmapResult, uint* innerkey, uint* dest, uint keystart,
			uint keystop, uint dstart);
	virtual ~GatherThread();
	void run();

	static uint gather(ulong* bitmap, uint bitmapSize, uint* probe,
			uint probeSize, uint* gathered);
};

#endif /* SRC_JOIN_CSTEP_GATHERTHREAD_H_ */
