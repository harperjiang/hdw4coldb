/*
 * StepCHTJoin.h
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_CSTEP_STEPCHTJOIN_H_
#define SRC_JOIN_CSTEP_STEPCHTJOIN_H_

#include "../Join.h"

class StepCHTJoin: public Join {
protected:
	Lookup* createLookup();
	const char* name();
protected:
	// Collect (remove zero) result after each step
	bool collectAfterFilter;
	bool collectAfterCht;

	virtual void init()=0;

	uint* chtInput = NULL;
	uint chtInputSize = 0;
	virtual void filter()=0;

	uint* chtResult = NULL;
	uint* hashInput = NULL;
	uint chtResultSize = 0;
	uint hashInputSize = 0;
	virtual void scanCht()=0;

	uint* hashResult = NULL;
	uint hashResultSize = 0;
	virtual void scanHash()=0;

	virtual void collect()=0;
public:
	StepCHTJoin(bool = false, bool = false, bool = false);
	virtual ~StepCHTJoin();

	virtual void join(kvlist* outer, kvlist* inner);
};

#endif /* SRC_JOIN_CSTEP_STEPCHTJOIN_H_ */
