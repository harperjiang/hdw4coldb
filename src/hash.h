/*
 * Hash table using linear probing
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#ifndef SRC_HASH_H_
#define SRC_HASH_H_

#include <stdint.h>

//#define NODE_LINK

#define PAYLOAD_SIZE 4
#define RATIO 1.5

/**
 * Data structure for key value pair
 */
typedef struct _kv {
	uint32_t key;
	uint8_t payload[PAYLOAD_SIZE];
} kv;

typedef void (*scanfunc)(uint32_t, uint8_t*, uint8_t*);

typedef struct _scan_context {
	uint8_t* inner;
	scanfunc func;
} scan_context;

/**
 * Data structure for hash table
 */
typedef struct _entry {
	uint32_t key;
	uint8_t payload[PAYLOAD_SIZE];
#ifdef NODE_LINK
	// TODO This pointer can be replaced with an offset bit to save memory
	struct _entry* next;
#endif
} entry;

typedef struct _hashtable {
	uint32_t size;
	uint32_t bucket_size;
	entry *buckets;
} hashtable;

void hash_build(hashtable* ht, kv* datas, uint32_t bucket_size);
void hash_init(hashtable* ht, uint32_t bucket_size);
// get first entry
entry* hash_get(hashtable* ht, uint32_t key);
void hash_scan(hashtable* ht, uint32_t key, scan_context* context);
void hash_put(hashtable* ht, uint32_t key, uint8_t *value);
uint32_t hash_size(hashtable* ht);
void hash_organize(hashtable* ht);
void hash_free(hashtable* ht);
#endif /* SRC_HASH_H_ */
