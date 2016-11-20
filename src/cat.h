/*
 * cat.h
 *
 *  Created on: Nov 19, 2016
 *      Author: harper
 */

#ifndef SRC_CAT_H_
#define SRC_CAT_H_

#include <stdint.h>
#include <stdbool.h>
#include "hash.h"

typedef struct _cat {
	uint64_t* 	bitmap;
	uint32_t	bitmap_size;
	uint8_t* 	payloads;
	uint32_t 	payload_size;
	hashtable* 	overflow;
} cat;

void 		cat_build(cat* cat, entry* datas, uint32_t size);
bool		cat_has(cat* cat, uint32_t key);
uint8_t* 	cat_find_uniq(cat* cat, uint32_t key);
void		cat_scan(cat* cat, uint32_t key, void (*scanfunc)(uint32_t,uint8_t*));
void		cat_free(cat* cat);

#endif /* SRC_CAT_H_ */
