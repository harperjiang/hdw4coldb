/*
 * Partition.h
 *
 *  Created on: Dec 22, 2016
 *      Author: Cathy
 */

#ifndef SRC_PART_PARTITION_H_
#define SRC_PART_PARTITION_H_

#include <sys/types.h>
#include "../lookup/Lookup.h"

class Partition {
private:
	kvlist* inner;
	kvlist* outer;
public:
	Partition();
	virtual ~Partition();

	kvlist* getInner();
	kvlist* getOuter();
};

class Partitioner {
public:
	Partitioner();
	virtual ~Partitioner();

	virtual Partition* part(kvlist* inner, kvlist* outer, uint num) = 0;
};

#endif /* SRC_PART_PARTITION_H_ */
