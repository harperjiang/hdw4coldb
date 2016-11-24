/*
 * hash.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "hash.h"

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "util.h"

void hash_build(hashtable* ht, kv* entries, uint32_t size) {
	ht->size = 0;
	ht->bucket_size = size * RATIO;
	ht->buckets = (kv*) calloc(ht->bucket_size, sizeof(kv));

	for (uint32_t i = 0; i < size; i++) {
		hash_put(ht, entries[i].key, entries[i].payload);
	}
}

void hash_init(hashtable* ht, uint32_t size) {
	ht->size = 0;
	ht->bucket_size = size;
	ht->buckets = (kv*) calloc(ht->bucket_size, sizeof(kv));
}

kv* hash_get(hashtable* ht, uint32_t key) {
	// No zero key is allowed
	assert(key != 0);
	if (ht->size == 0)
		return NULL ;
	uint32_t hval = hash(key) % ht->bucket_size;
	kv* bucket = ht->buckets + hval;

	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
	if (bucket->key == key) {
		return bucket;
	}

	return NULL ;
}

void hash_scan(hashtable* ht, uint32_t key, scan_context* context) {
	assert(key != 0);
	if (ht->size == 0)
		return;
	uint32_t hval = hash(key) % ht->bucket_size;
	kv* bucket = ht->buckets + hval;

	while (bucket->key != 0) {
		if (bucket->key == key) {
			context->func(bucket->key, bucket->payload, context->inner,
					context->params);
		}
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}
}

void hash_put(hashtable* ht, uint32_t key, uint8_t *value) {
	// No zero key
	assert(key != 0);
	if (ht->size * RATIO > ht->bucket_size) {
		hash_organize(ht);
	}

	uint32_t hval = hash(key) % ht->bucket_size;
	kv* bucket = ht->buckets + hval;

	while (bucket->key != 0) {
		hval = (hval + 1) % ht->bucket_size;
		bucket = ht->buckets + hval;
	}

	ht->buckets[hval].key = key;
	memcpy(ht->buckets[hval].payload, value, sizeof(uint8_t) * PAYLOAD_SIZE);

	ht->size += 1;
}

uint32_t hash_size(hashtable* ht) {
	return ht->size;
}

void hash_organize(hashtable* ht) {
	uint32_t newbktsize = ht->bucket_size * RATIO;

	kv* newbkts = (kv*) calloc(newbktsize, sizeof(kv));

	kv* oldbkts = ht->buckets;
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
/**===================================================================
 * Create class information for Hash
 ======================================================================*/

void hash_algo_build(const algo* self, kv* datas, uint32_t size) {
	hash_build((hashtable*) self, datas, size);
}

uint8_t* hash_algo_access(const algo* self, uint32_t key) {
	return hash_get((hashtable*) self, key)->payload;
}

void hash_algo_scan(const algo* self, uint32_t key, scan_context* context) {
	hash_scan((hashtable*) self, key, context);
}

void hash_algo_free(const algo* self) {
	hash_free((hashtable*) self);
}

algo_class hash_class;

void hash_init_class() {
	hash_class.name = "hash";
	hash_class.build = hash_algo_build;
	hash_class.access = hash_algo_access;
	hash_class.scan = hash_algo_scan;
	hash_class.free = hash_algo_free;
}

algo* hash_algo_new() {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	ht->prototype = &hash_class;
	return (algo*) ht;
}
