/*
 * hash.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "hash.h"
#include "util.h"
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#define RATIO 1.5

void hash_build(hashtable* ht, uint32_t bucket_size) {
	ht->size = 0;
	ht->buckets = (hashnode*) calloc(bucket_size, sizeof(hashnode));
	ht->bucket_size = bucket_size;
}

uint8_t* hash_get(hashtable* ht, uint32_t key) {
	// No zero key is allowed
	key += 1;
	uint32_t hval = hash(key) % ht->bucket_size;
	hashnode* bucket = ht->buckets + hval;
	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
	if (bucket->key == key) {
		return bucket->payload;
	}
	return NULL;
}

void hash_put(hashtable* ht, uint32_t key, uint8_t *value) {
	// No zero key
	key += 1;
	uint32_t hval = hash(key) % ht->bucket_size;
	hashnode* bucket = ht->buckets + hval;
	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
	if (bucket->key == 0) {
		// Not found
		ht->buckets[hval].key = key;
		memcpy(ht->buckets[hval].payload, value,
				sizeof(uint8_t) * PAYLOAD_SIZE);
		ht->size += 1;
	} else {
		// Found, overwrite
		memcpy(bucket->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
	}
}

uint32_t hash_size(hashtable* ht) {
	return ht->size;
}

void hash_organize(hashtable* ht) {
	uint32_t newbktsize = ht->size * RATIO;

	hashnode* newbkts = (hashnode*) calloc(newbktsize, sizeof(hashnode));

	for (uint32_t i = 0; i < ht->bucket_size; i++) {
		hashnode node = ht->buckets[i];
		if (node.key != 0) {
			uint32_t offset = hash(node.key) % newbktsize;
			while (newbkts[offset].key != 0) {
				offset++;
			}
			memcpy(newbkts + offset, ht->buckets + i, sizeof(hashnode));
		}
	}
	ht->bucket_size = newbktsize;
	ht->buckets = newbkts;
}
