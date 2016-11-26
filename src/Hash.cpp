/*
 * Hash.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */
#include <cassert>
#include <cstring>

#include "Hash.h"
#include "util.h"

Hash::Hash() :
		Lookup("Hash") {
	this->bucket_size = 0;
	this->buckets = NULL;
	this->_size = 0;
}

Hash::Hash(uint32_t size) :
		Hash() {
	this->bucket_size = size;
	this->buckets = new kv[this->bucket_size]();
}

Hash::~Hash() {
	delete[] buckets;
}

void Hash::build(kv* entries, uint32_t size) {
	this->_size = 0;
	this->bucket_size = size * RATIO;
	this->buckets = new kv[this->bucket_size]();

	for (uint32_t i = 0; i < size; i++) {
		this->put(entries[i].key, entries[i].payload);
	}
}

uint32_t Hash::size() {
	return _size;
}

uint32_t Hash::bucketSize() {
	return bucket_size;
}

uint8_t* Hash::access(uint32_t key) {
	kv* result = this->get(key);
	if (NULL == result) {
		return NULL;
	}
	return result->payload;
}

void Hash::scan(uint32_t key, ScanContext* context) {
	assert(key != 0);
	if (this->_size == 0)
		return;
	uint32_t hval = hash(key) % this->bucket_size;
	kv* bucket = this->buckets + hval;

	while (bucket->key != 0) {
		if (bucket->key == key) {
			context->execute(bucket->key, bucket->payload);
		}
		hval = (hval + 1) % this->bucket_size;
		bucket = this->buckets + hval;
	}
}

void Hash::internalPut(uint32_t key, uint8_t* payload) {
	uint32_t hval = hash(key) % this->bucket_size;
	kv* bucket = this->buckets + hval;

	while (bucket->key != 0) {
		hval = (hval + 1) % this->bucket_size;
		bucket = this->buckets + hval;
	}

	this->buckets[hval].key = key;
	::memcpy(this->buckets[hval].payload, payload,
			sizeof(uint8_t) * PAYLOAD_SIZE);

	this->_size += 1;
}

void Hash::put(uint32_t key, uint8_t* payload) {
// No zero key
	assert(key != 0);
	if (this->_size * RATIO > this->bucket_size) {
		this->organize(this->bucket_size * RATIO);
	}
	internalPut(key, payload);
}

kv* Hash::get(uint32_t key) {
// No zero key is allowed
	assert(key != 0);
	if (this->_size == 0)
		return NULL;
	uint32_t hval = hash(key) % this->bucket_size;
	kv* bucket = this->buckets + hval;

	while (bucket->key != 0 && bucket->key != key) {
		hval = (hval + 1) % bucket_size;
		bucket = this->buckets + hval;
	}
	if (bucket->key == key) {
		return bucket;
	}

	return NULL;
}

bool Hash::has(uint32_t key) {
	return get(key) != NULL;
}

void Hash::organize(uint32_t newbktsize) {
	kv* newbkts = new kv[newbktsize]();

	kv* oldbkts = this->buckets;
	uint32_t oldbktsize = this->bucket_size;

	this->bucket_size = newbktsize;
	this->buckets = newbkts;
	this->_size = 0;

	for (uint32_t i = 0; i < oldbktsize; i++) {
		if (oldbkts[i].key != 0)
			this->internalPut(oldbkts[i].key, oldbkts[i].payload);
	}
	delete[] oldbkts;
}
