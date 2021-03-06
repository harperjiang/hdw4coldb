/*
 * CHTJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_SIMPLE_CHTJOIN_H_
#define SRC_JOIN_SIMPLE_CHTJOIN_H_

#include "LookupJoin.h"

class CHTJoin: public LookupJoin {
public:
	CHTJoin(uint = 0, bool = false);
	virtual ~CHTJoin();

protected:
	const char* name();
	Lookup* createLookup();
};

#endif /* SRC_JOIN_SIMPLE_CHTJOIN_H_ */
