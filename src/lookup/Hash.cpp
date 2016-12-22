/*
 * Hash.cpp
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */
#include <cassert>
#include <cstring>

#include "Hash.h"

#include "LookupHelper.h"

Hash::Hash() :
		Lookup("Hash") {
	this->bucket_size = 0;
	this->buckets = NULL;
	this->payloads = NULL;
	this->_size = 0;
}

Hash::Hash(uint32_t size) :
		Hash() {
	this->bucket_size = size;
	this->buckets = new uint32_t[this->bucket_size]();
	this->payloads = new uint8_t[this->bucket_size * PAYLOAD_SIZE];
}

Hash::~Hash() {
	delete[] buckets;
	delete[] payloads;
}

void Hash::build(kv* entries, uint32_t size) {
	this->_size = 0;
	this->bucket_size = size * RATIO;
	this->buckets = new uint32_t[this->bucket_size]();
	this->payloads = new uint8_t[this->bucket_size * PAYLOAD_SIZE];

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
	return this->get(key);
}

void Hash::scan(uint32_t key, ScanContext* context) {
	assert(key != 0);
	if (this->_size == 0)
		return;
	uint32_t hval = mut_hash(key) % this->bucket_size;

	while (buckets[hval] != 0) {
		if (buckets[hval] == key) {
			context->execute(key, payloads + hval * PAYLOAD_SIZE);
		}
		hval = (hval + 1) % this->bucket_size;
	}
}

void Hash::internalPut(uint32_t key, uint8_t* payload) {
	uint32_t hval = mut_hash(key) % this->bucket_size;

	while (buckets[hval] != 0) {
		hval = (hval + 1) % this->bucket_size;
	}

	this->buckets[hval] = key;
	::memcpy(this->payloads + hval * PAYLOAD_SIZE, payload,
			sizeof(uint8_t) * PAYLOAD_SIZE);

	this->_size += 1;
}

void Hash::put(uint32_t key, uint8_t* payload) {
	// No zero key
	if (key == 0)
		return;
	if (this->_size * RATIO > this->bucket_size) {
		this->organize(this->bucket_size * RATIO);
	}
	internalPut(key, payload);
}

uint8_t* Hash::get(uint32_t key) {
	// No zero key is allowed
	if (key == 0)
		return NULL;
	if (this->_size == 0)
		return NULL;
	uint32_t hval = mut_hash(key) % this->bucket_size;

	while (buckets[hval] != 0 && buckets[hval] != key) {
		hval = (hval + 1) % bucket_size;
	}

	return buckets[hval] == key ? payloads + hval * PAYLOAD_SIZE : NULL;
}

bool Hash::has(uint32_t key) {
	return get(key) != NULL;
}

void Hash::organize(uint32_t newbktsize) {
	uint32_t* newbkts = new uint32_t[newbktsize]();
	uint8_t* newplds = new uint8_t[newbktsize * PAYLOAD_SIZE];

	uint32_t* oldbkts = this->buckets;
	uint8_t* oldplds = this->payloads;
	uint32_t oldbktsize = this->bucket_size;

	this->bucket_size = newbktsize;
	this->buckets = newbkts;
	this->payloads = newplds;
	this->_size = 0;

	for (uint32_t i = 0; i < oldbktsize; i++) {
		if (oldbkts[i] != 0)
			this->internalPut(oldbkts[i], oldplds + i * PAYLOAD_SIZE);
	}
	delete[] oldbkts;
	delete[] oldplds;
}
