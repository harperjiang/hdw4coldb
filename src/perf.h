/*
 * perf.h
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#ifndef SRC_PERF_H_
#define SRC_PERF_H_

#include "cht.h"
#include "hash.h"

typedef struct _kvlist {
	kv* entries;
	uint32_t size;
} kvlist;

void perf_loadkey(const char* filename, kvlist* result);

#endif /* SRC_PERF_H_ */
