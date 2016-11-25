/*
 * timer.c
 *
 *  Created on: Nov 23, 2016
 *      Author: harper
 */

#include "Timer.h"

Timer::Timer() {

}

Timer::~Timer() {

}

void Timer::start() {
	gettimeofday(&_start, NULL);
}

void Timer::stop() {
	struct timeval stoptime;
	gettimeofday(&stoptime, NULL);

	_wallclockms = 1000 * (stoptime.tv_sec - _start.tv_sec)
			+ (stoptime.tv_usec - _start.tv_usec) / 1000;
}

uint64_t Timer::wallclockms() {
	return _wallclockms;
}
