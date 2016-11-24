/*
 * Concise Hash Table
 *
 *  Created on: Nov 15, 2016
 *      Author: harper
 */

#ifndef CHT_H_
#define CHT_H_

#include <stdint.h>
#include <stdbool.h>

#include "hash.h"

#define THRESHOLD 5

typedef struct _cht_entry {
	uint32_t 		key;
	uint8_t 		payload[PAYLOAD_SIZE];
} cht_entry;

typedef struct _cht {
	algo_class* prototype;
	uint64_t* 	bitmap;
	uint32_t	bitmap_size;
	cht_entry* 	payloads;
	uint32_t 	payload_size;
	hashtable* 	overflow;
} cht;

void 		cht_build(cht* cht, kv* datas, uint32_t size);
bool		cht_has(cht* cht, uint32_t key);
cht_entry* 	cht_find_uniq(cht* cht, uint32_t key);
void		cht_scan(cht* cht, uint32_t key, scan_context* context);
void		cht_free(cht* cht);

algo*		cht_algo_new();
#endif /* CHT_H_ */
