/*
 * Thread.h
 *
 *  Created on: Dec 17, 2016
 *      Author: Hao Jiang
 */

#ifndef SRC_UTIL_THREAD_H_
#define SRC_UTIL_THREAD_H_

#include <pthread.h>

class Thread {
private:
	pthread_t handle;
public:
	Thread();
	virtual ~Thread();

	friend void* __thread_run(void*);

	void start();
	void wait();

	virtual void run() = 0;

};

#endif /* SRC_UTIL_THREAD_H_ */
