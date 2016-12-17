/*
 * Thread.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: Cathy
 */

#include "Thread.h"

void* __thread_run(void* input) {
	Thread* thread = (Thread*) input;
	thread->run();
	return NULL;
}

Thread::Thread() {
}

Thread::~Thread() {
}

void Thread::start() {
	pthread_create(&handle, NULL, __thread_run, this);
}

void Thread::wait() {
	void* result;
	pthread_join(handle, &result);
}

