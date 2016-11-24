/*
 * algo.h
 *
 *  Created on: Nov 23, 2016
 *      Author: Cathy
 */

#ifndef SRC_ALGO_H_
#define SRC_ALGO_H_

#include <stdint.h>
#include <stdlib.h>

#define PAYLOAD_SIZE 4
/**
 * Data structure for key value pair
 */
typedef struct _kv {
	uint32_t key;
	uint8_t payload[PAYLOAD_SIZE];
} kv;

typedef void (*scanfunc)(uint32_t, uint8_t*, uint8_t*, void* params);

typedef struct _scan_context {
	uint8_t* inner;
	scanfunc func;
	void* params;
} scan_context;

typedef struct _algo algo;

typedef struct _algo_class {
	const char* name;
	void (*build)(const algo* self, kv* datas, uint32_t size);
	uint8_t* (*access)(const algo* self, uint32_t key);
	void (*scan)(const algo* self, uint32_t key, scan_context* context);
	void (*free)(const algo* self);
} algo_class;

typedef struct _algo {
	algo_class *prototype;
} algo;

void init_class();

#endif /* SRC_ALGO_H_ */
