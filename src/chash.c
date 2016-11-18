/*
 * hash.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "chash.h"
#include "util.h"
#include <memory.h>
#include <stdlib.h>


void chash_build(chashtable* ht, uint32_t bucket_size) {
	ht->size = 0;
	ht->buckets = (cnode*) calloc(bucket_size, sizeof(cnode));
	ht->bucket_size = bucket_size;
}

uint8_t* chash_get(chashtable* ht, uint32_t key) {
	uint32_t hval = hash(key) % ht->bucket_size;
	cnode* bucket = ht->buckets + hval;
	while (bucket->key != 0 && bucket->key != key) {
		bucket = bucket->next;
	}
	if (bucket->key == key) {
		return bucket->payload;
	}
	return NULL;
}

void chash_put(chashtable* ht, uint32_t key, uint8_t *value) {
	uint32_t hval = hash(key) % ht->bucket_size;
	cnode* bucket = ht->buckets + hval;
	cnode* lastbucket = NULL;
	while (bucket->key != 0 && bucket->key != key) {
		if (bucket->next->key == 0)
			lastbucket = bucket;
		bucket = bucket->next;
	}
	if (bucket->key == 0) {
		// Not found
		cnode* newnode = (cnode*) malloc(sizeof(cnode));
		newnode->key = key;
		newnode->payload = (uint8_t*) malloc(sizeof(uint8_t) * PAYLOAD_SIZE);
		memcpy(newnode->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
		if (lastbucket->key != 0)
			lastbucket->next = newnode;
		else
			ht->buckets[hval] = *newnode;
		ht->size += 1;
	} else {
		// Found, overwrite
		memcpy(bucket->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
	}
}

uint32_t chash_size(chashtable* ht) {
	return ht->size;
}
