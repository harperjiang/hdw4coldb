/*
 * LookupJoin.h
 *
 * Single-Thread join use a lookup table
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_LOOKUPJOIN_H_
#define SRC_JOIN_LOOKUPJOIN_H_

#include "../Join.h"
#include "../../lookup/Lookup.h"

class LookupJoin: public Join {
protected:
	uint numThread = 0;
protected:
	void joinSingleThread();
	void joinMultiThread();
public:
	LookupJoin(uint = 0, bool = false);
	virtual ~LookupJoin();

	void join(kvlist* outer, kvlist* inner);

};

#endif /* SRC_JOIN_LOOKUPJOIN_H_ */
