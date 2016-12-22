/*
 * CounterThread.cpp
 *
 *  Created on: Dec 20, 2016
 *      Author: harper
 */

#include "CounterThread.h"

CounterThread::CounterThread(uint* base, uint begin, uint end, uint value,
		bool equal) {
	// TODO Auto-generated constructor stub

}

CounterThread::~CounterThread() {
	// TODO Auto-generated destructor stub
}

void CounterThread::run() {

}

uint CounterThread::count(uint* base, uint length, uint numThread, uint value,
		bool equal) {
	uint result = 0;
	for (uint i = 0; i < length; i++) {
		result += ((base[i] == value) == equal);
	}
	return result;
}
