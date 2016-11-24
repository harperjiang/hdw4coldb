/*
 * Hash table using linear probing
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#ifndef SRC_HASH_H_
#define SRC_HASH_H_

#include <stdint.h>
#include "algo.h"

#define RATIO 1.5

typedef struct _hashtable {
	algo_class* prototype;
	uint32_t size;
	uint32_t bucket_size;
	kv *buckets;
} hashtable;

void 	hash_build(hashtable* ht, kv* datas, uint32_t bucket_size);
void 	hash_init(hashtable* ht, uint32_t bucket_size);
kv* 	hash_get(hashtable* ht, uint32_t key);
void 	hash_scan(hashtable* ht, uint32_t key, scan_context* context);
void	hash_put(hashtable* ht, uint32_t key, uint8_t *value);
uint32_t hash_size(hashtable* ht);
void 	hash_organize(hashtable* ht);
void 	hash_free(hashtable* ht);

algo* hash_algo_new();

#endif /* SRC_HASH_H_ */
