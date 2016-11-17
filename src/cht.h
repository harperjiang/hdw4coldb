/*
 * Concise Hash Table
 *
 *  Created on: Nov 15, 2016
 *      Author: harper
 */

#ifndef CHT_H_
#define CHT_H_

#include <stdint.h>
#include "hash.h"

#define THRESHOLD 5

typedef struct _data {
	uint32_t 	key;
	uint8_t 	payload[PAYLOAD_SIZE];
} data;

typedef struct _cht {
	uint64_t* 	bitmap;
	uint32_t	bitmap_size;
	data* 		payloads;
	uint32_t 	payload_size;
	hashtable* 	overflow;
} cht;

void 		cht_build(cht* cht, data* datas, uint32_t size);
uint8_t* 	cht_find(cht*cht, uint32_t key);
#endif /* CHT_H_ */
