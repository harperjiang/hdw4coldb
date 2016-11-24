/*
 *  CAT use a large bitmap (equal to the given key range)
 *  Created on: Nov 19, 2016
 *      Author: harper
 */

#ifndef SRC_CAT_H_
#define SRC_CAT_H_

#include <stdint.h>
#include <stdbool.h>

#include "algo.h"
#include "hash.h"

typedef struct _cat {
	uint32_t 	min;
	uint32_t 	max;
	uint64_t* 	bitmap;
	uint32_t	bitmap_size;
	uint8_t* 	payloads;
	uint32_t 	payload_size;
} cat;

void 	cat_build(cat* cat, kv* datas, uint32_t size);
bool	cat_has(cat* cat, uint32_t key);
uint8_t* 	cat_find_uniq(cat* cat, uint32_t key);
void	cat_free(cat* cat);

#endif /* SRC_CAT_H_ */
