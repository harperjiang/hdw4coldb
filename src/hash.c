/*
 * hash.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "hash.h"
#include <stdint.h>
#include <memory.h>

uint32_t hash(uint32_t v) {
	return v * UINT32_C(2654435761);
}

void hash_build(hashtable* ht, uint32_t bucket_size) {
	ht->size = 0;
	ht->buckets = (hashnode*) malloc(sizeof(hashnode) * bucket_size);
	ht->bucket_size = bucket_size;
}

uint8_t* hash_get(hashtable* ht, uint32_t key) {
	uint32_t hval = hash(key) % ht->bucket_size;
	hashnode* bucket = ht->buckets + hval;
	while (bucket != NULL && bucket->key != key) {
		bucket = bucket->next;
	}
	if (bucket != NULL && bucket->key == key) {
		return bucket->payload;
	}
	return NULL;
}

void hash_put(hashtable* ht, uint32_t key, uint8_t *value) {
	uint32_t hval = hash(key) % ht->bucket_size;
	hashnode* bucket = ht->buckets + hval;
	hashnode* lastbucket = NULL;
	while (bucket != NULL && bucket->key != key) {
		if (bucket->next == NULL)
			lastbucket = bucket;
		bucket = bucket->next;
	}
	if (bucket == NULL) {
		// Not found
		hashnode* newnode = (hashnode*) malloc(sizeof(hashnode));
		newnode->key = key;
		newnode->payload = (uint8_t*) malloc(sizeof(uint8_t) * PAYLOAD_SIZE);
		memcpy(newnode->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
		if (lastbucket != NULL)
			lastbucket->next = newnode;
		else
			ht->buckets[hval] = *newnode;
		ht->size += 1;
	} else {
		// Found, overwrite
		memcpy(bucket->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
	}
}

uint32_t hash_size(hashtable* ht) {
	return ht->size;
}
