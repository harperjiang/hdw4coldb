/*
 * BloomFilter.h
 *
 *  Created on: Dec 26, 2016
 *      Author: Cathy
 */

#ifndef SRC_LOOKUP_BLOOMFILTER_H_
#define SRC_LOOKUP_BLOOMFILTER_H_

#include <sys/types.h>
#include "Lookup.h"

class BloomFilter {
private:
	uint size;
	uint* factors;
	ulong* bitmap;
	uint bitmapSize;
public:
	BloomFilter(uint size, uint* factors);
	virtual ~BloomFilter();

	void build(kv* datas, uint size);
	bool test(uint key);

	void stat();
};

#endif /* SRC_LOOKUP_BLOOMFILTER_H_ */
