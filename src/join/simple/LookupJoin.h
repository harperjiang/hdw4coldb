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
	Lookup* _lookup;
public:
	LookupJoin();
	virtual ~LookupJoin();

	void join(kvlist* outer, kvlist* inner, bool enableProfiling);

protected:
	virtual Lookup* buildLookup(kvlist* outer) = 0;
};

#endif /* SRC_JOIN_LOOKUPJOIN_H_ */
