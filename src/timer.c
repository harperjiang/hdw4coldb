/*
 * timer.c
 *
 *  Created on: Nov 23, 2016
 *      Author: harper
 */

#include "timer.h"

void timer_start(timer_token* token) {
	gettimeofday(&token->start, NULL);
}

void timer_stop(timer_token* token) {
	struct timeval stoptime;
	gettimeofday(&stoptime, NULL);

	token->wallclockms = 1000 * (stoptime.tv_sec - token->start.tv_sec)
			+ (stoptime.tv_usec - token->start.tv_usec) / 1000;
}

