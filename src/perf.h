/*
 * perf.h
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#ifndef SRC_PERF_H_
#define SRC_PERF_H_

#define INNER 20000000
#define OUTER 500000000

#include "hash.h"
#include "cht.h"

void perf_loadcht(cht* table, const char* filename);
void perf_loadhash(hashtable* table, const char* filename);

void perf_accesscht(cht* table, uint32_t size);
void perf_accesshash(hashtable* table, uint32_t size);
#endif /* SRC_PERF_H_ */
