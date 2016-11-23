/*
 * timer.h
 *
 *  Created on: Nov 23, 2016
 *      Author: harper
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include <time.h>

typedef struct _timer_token {
	struct timeval 	start;
	uint64_t 		wallclockms;
} timer_token;

void timer_start(timer_token* token);
void timer_stop(timer_token* token);

#endif /* SRC_TIMER_H_ */
