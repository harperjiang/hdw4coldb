/*
 * CHT.h
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */

#ifndef SRC_CPP_CHT_H_
#define SRC_CPP_CHT_H_

#include "Hash.h"
#include "Lookup.h"
#include "BloomFilter.h"

class CHT: public Lookup {
public:
	uint64_t* bitmap;
	uint32_t bitmap_size;
	uint32_t* keys;
	uint8_t* payloads;
	uint32_t payload_size;
	Hash* overflow;
	BloomFilter* bf;
public:
	CHT();
	virtual ~CHT();

	bool has(uint32_t key);
	void build(kv* datas, uint32_t size);
	uint8_t* access(uint32_t key);
	void scan(uint32_t key, ScanContext* context);

	uint8_t* findUnique(uint32_t key);
	uint32_t payloadSize();
	uint32_t bitmapSize();
	Hash* getOverflow();
};

#endif /* SRC_CPP_CHT_H_ */
