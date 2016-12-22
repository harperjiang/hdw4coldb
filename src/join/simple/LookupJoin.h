/*
 * LookupJoin.h
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
	Matched* _matched;
public:
	LookupJoin();
	virtual ~LookupJoin();

	virtual void join(kvlist* outer, kvlist* inner, uint split,
			bool enableProfiling);
protected:
	virtual Lookup* buildLookup(kvlist* outer) = 0;
	virtual Matched* getMatched();
};

#endif /* SRC_JOIN_LOOKUPJOIN_H_ */
