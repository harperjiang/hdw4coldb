/*
 * OclHashJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_OCL_OCLHASHJOIN_H_
#define SRC_JOIN_OCL_OCLHASHJOIN_H_

#include "../Join.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"

class OclHashJoin: public Join {
protected:
	CLEnv* env;
	CLProgram* hashScan;

protected:
	Lookup* createLookup();
public:
	OclHashJoin(bool = false);
	virtual ~OclHashJoin();
	void join(kvlist* outer, kvlist* inner);
};

#endif /* SRC_JOIN_OCL_OCLHASHJOIN_H_ */
