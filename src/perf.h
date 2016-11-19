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

uint32_t* perf_loadkey(const char* filename, uint32_t* sizeholder);

void perf_buildcht(cht* table, const char* filename);
void perf_buildhash(hashtable* table, const char* filename);

void perf_cht_access(cht* table, uint32_t size, uint32_t* keys);
void perf_hash_access(hashtable* table, uint32_t size, uint32_t* keys);

void perf_cht_scan(cht* table, uint32_t size, uint32_t* keys, void (*scanfunc)(cht_entry*));
void perf_hash_scan(hashtable* table, uint32_t size, uint32_t* keys, void(*scanfunc)(entry*));
#endif /* SRC_PERF_H_ */
