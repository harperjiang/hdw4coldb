/*
 * timer.c
 *
 *  Created on: Nov 23, 2016
 *      Author: harper
 */

#include <memory.h>
#include "Timer.h"

Timer::Timer() {
	intervalLimit = 10;
	intervals = new uint64_t[intervalLimit];
	names = new char*[intervalLimit];
}

Timer::~Timer() {
	delete[] intervals;
	delete[] names;
}

void Timer::start() {
	_wallclockms = 0;
	intervalCounter = 0;
	gettimeofday(&_start, NULL);
}

void Timer::stop() {
	this->pause();
}

uint64_t Timer::pause() {
	struct timeval stoptime;
	gettimeofday(&stoptime, NULL);

	if (intervalCounter == intervalLimit) {
		intervalLimit *= 2;
		uint64_t* newIntervals = new uint64_t[intervalLimit];
		::memcpy(newIntervals, intervals, sizeof(uint64_t) * intervalCounter);
		delete[] intervals;
		intervals = newIntervals;
	}

	uint64_t interval = 1000 * (stoptime.tv_sec - _start.tv_sec)
			+ (stoptime.tv_usec - _start.tv_usec) / 1000;
	_wallclockms += interval;
	return interval;
}

void Timer::resume() {
	gettimeofday(&_start, NULL);
}

void Timer::interval(const char* name) {
	uint64_t interval = pause();
	intervals[intervalCounter] = interval;
	names[intervalCounter++] = (char*) name;
	resume();
}

uint64_t Timer::wallclockms() {
	return _wallclockms;
}

uint32_t Timer::numInterval() {
	return this->intervalCounter;
}

uint64_t Timer::interval(uint32_t index) {
	return this->intervals[index];
}

char* Timer::name(uint32_t index) {
	return this->names[index];
}
