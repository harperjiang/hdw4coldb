/*
 * HashJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_SIMPLE_HASHJOIN_H_
#define SRC_JOIN_SIMPLE_HASHJOIN_H_

#include "LookupJoin.h"

class HashJoin: public LookupJoin {
public:
	HashJoin(bool = false);
	virtual ~HashJoin();

	Lookup* createLookup();
};

#endif /* SRC_JOIN_SIMPLE_HASHJOIN_H_ */