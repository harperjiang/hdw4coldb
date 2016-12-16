/*
 * timer.h
 *
 *  Created on: Nov 23, 2016
 *      Author: harper
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

class Timer {
private:
	struct timeval _start;
	uint64_t _wallclockms = 0;

public:
	Timer();
	virtual ~Timer();

	void start();
	void pause();
	void resume();
	void stop();

	uint64_t wallclockms();
};

#endif /* SRC_TIMER_H_ */
