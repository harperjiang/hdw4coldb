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
	HashJoin(uint = 0, bool = false);
	virtual ~HashJoin();
protected:
	Lookup* createLookup();
	const char* name();
};

#endif /* SRC_JOIN_SIMPLE_HASHJOIN_H_ */
