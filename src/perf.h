/*
 * perf.h
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#ifndef SRC_PERF_H_
#define SRC_PERF_H_

#include "hash.h"
#include "cht.h"

uint32_t* 	perf_loadkey(const char* filename, uint32_t* sizeholder);

void 		perf_buildcht(cht* table, const char* filename);
void 		perf_buildhash(hashtable* table, const char* filename);

void		perf_scancht(cht* table, kv* workload, uint32_t size, void (*scanfunc)(uint32_t, uint8_t*, uint8_t*,void*));
void		perf_scanhash(hashtable* table, kv* workload, uint32_t size, void (*scanfunc)(uint32_t, uint8_t*, uint8_t*,void*));

#endif /* SRC_PERF_H_ */
