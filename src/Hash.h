/*
 * Hash.h
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */

#ifndef SRC_CPP_HASH_H_
#define SRC_CPP_HASH_H_

#include "Lookup.h"

#define RATIO 1.5

class Hash: public Lookup {
private:
	uint32_t _size;
	uint32_t bucket_size;
	kv *buckets;
public:
	Hash();
	Hash(uint32_t size);
	virtual ~Hash();

	bool has(uint32_t key);
	void build(kv* datas, uint32_t size);
	uint8_t* access(uint32_t key);
	void scan(uint32_t key, ScanContext* context);

	kv* get(uint32_t key);
	void put(uint32_t key, uint8_t* payload);
	uint32_t size();
	uint32_t bucketSize();
	void organize(uint32_t newsize);
protected:
	void internalPut(uint32_t key, uint8_t* payload);
};

#endif /* SRC_CPP_HASH_H_ */
