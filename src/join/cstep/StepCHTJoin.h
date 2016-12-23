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

protected:
	// Collect (remove zero) result after each step
	bool collectAfterFilter;
	bool collectAfterCht;

	virtual void init()=0;

	uint* chtInput;
	uint chtInputSize;
	virtual void filter()=0;

	uint* chtResult;
	uint* hashInput;
	uint chtResultSize;
	uint hashInputSize;
	virtual void scanCht()=0;

	uint* hashResult;
	uint hashResultSize;
	virtual void scanHash()=0;

	virtual void collect()=0;
public:
	StepCHTJoin(bool = false, bool = false, bool = false);
	virtual ~StepCHTJoin();

	virtual void join(kvlist* outer, kvlist* inner);
};

#endif /* SRC_JOIN_CSTEP_STEPCHTJOIN_H_ */
