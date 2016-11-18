/*
 * hash.h
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#ifndef SRC_CHASH_H_
#define SRC_CHASH_H_

#include <stdint.h>

#define PAYLOAD_SIZE 4

/**
 * Data structure for hash table
 */
typedef struct _cnode {
	uint32_t 	key;
	uint8_t 	*payload;
	struct _cnode* 	next;
} cnode;

typedef struct _chashtable {
	uint32_t	size;
	uint32_t	bucket_size;
	cnode		*buckets;
} chashtable;

void 		chash_build(chashtable* ht, uint32_t bucket_size);
uint8_t* 	chash_get(chashtable* ht, uint32_t key);
void 		chash_put(chashtable* ht, uint32_t key, uint8_t *value);
uint32_t	chash_size(chashtable* ht);
#endif /* SRC_HASH_H_ */
