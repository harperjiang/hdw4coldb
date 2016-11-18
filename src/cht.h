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

typedef struct _cht {
	uint64_t* 	bitmap;
	uint32_t	bitmap_size;
	entry* 		payloads;
	uint32_t 	payload_size;
	hashtable* 	overflow;
} cht;

void 	cht_build(cht* cht, entry* datas, uint32_t size);
entry* 	cht_find(cht* cht, uint32_t key);
void	cht_free(cht* cht);
#endif /* CHT_H_ */
