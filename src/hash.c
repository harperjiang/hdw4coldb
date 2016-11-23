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
#include <assert.h>

void hash_build(hashtable* ht, kv* entries, uint32_t size) {
	ht->size = 0;
	ht->bucket_size = size * RATIO;
	ht->buckets = (entry*) calloc(ht->bucket_size, sizeof(entry));

	for (uint32_t i = 0; i < size; i++) {
		hash_put(ht, entries[i].key, entries[i].payload);
	}
}

void hash_init(hashtable* ht, uint32_t size) {
	ht->size = 0;
	ht->bucket_size = size;
	ht->buckets = (entry*) calloc(ht->bucket_size, sizeof(entry));
}

entry* hash_get(hashtable* ht, uint32_t key) {
	// No zero key is allowed
	assert(key != 0);
	uint32_t hval = hash(key) % ht->bucket_size;
	entry* bucket = ht->buckets + hval;

	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
	if (bucket->key == key) {
		return bucket;
	}

	return NULL;
}

void hash_scan(hashtable* ht, uint32_t key,
		void (*scanfunc)(uint32_t key, uint8_t* payload)) {
	assert(key != 0);
	uint32_t hval = hash(key) % ht->bucket_size;
	entry* bucket = ht->buckets + hval;
#ifndef NODE_LINK
	while (bucket->key != 0) {
		if (bucket->key == key) {
			scanfunc(bucket->key, bucket->payload);
		}
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
#else
	while(bucket->key != 0 && bucket->key!=key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
	if(bucket->key == 0) {
		return;
	}
	else {
		while(bucket != null) {
			scanfunc(bucket);
			bucket = bucket->next;
		}
	}
#endif
}

void hash_put(hashtable* ht, uint32_t key, uint8_t *value) {
	// No zero key
	assert(key != 0);
	if (ht->size * RATIO > ht->bucket_size) {
		hash_organize(ht);
	}

	uint32_t hval = hash(key) % ht->bucket_size;
	entry* bucket = ht->buckets + hval;

#ifndef NODE_LINK
	while (bucket->key != 0) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}

	ht->buckets[hval].key = key;
	memcpy(ht->buckets[hval].payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
#else
	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}

	if (bucket->key == 0) {
		// Found an empty slot
		ht->buckets[hval].key = key;
		memcpy(ht->buckets[hval].payload, value,
				sizeof(uint8_t) * PAYLOAD_SIZE);
	} else {
		entry* head = bucket;
		// Go with the cursor
		while (bucket->next != NULL) {
			bucket = bucket->next;
			assert(bucket != head);
		}
		entry* last = bucket;
		hval = (bucket - ht->buckets + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
		// Look for next slot
		while (bucket->key != 0) {
			hval = (hval + 1) % ht->bucket_size;
			bucket = ht->buckets + hval;
		}
		last->next = bucket;
		bucket->key = key;
		memcpy(bucket->payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);
	}
#endif
	ht->size += 1;
}

uint32_t hash_size(hashtable* ht) {
	return ht->size;
}

void hash_organize(hashtable* ht) {
	uint32_t newbktsize = ht->bucket_size * RATIO;

	entry* newbkts = (entry*) calloc(newbktsize, sizeof(entry));

	entry* oldbkts = ht->buckets;
	uint32_t oldbktsize = ht->bucket_size;

	ht->bucket_size = newbktsize;
	ht->buckets = newbkts;
	ht->size = 0;

	for (uint32_t i = 0; i < oldbktsize; i++) {
		if (oldbkts[i].key != 0)
			hash_put(ht, oldbkts[i].key, oldbkts[i].payload);
	}
}

void hash_free(hashtable* ht) {
	free(ht->buckets);
	free(ht);
}
