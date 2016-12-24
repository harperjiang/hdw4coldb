/*
 * OclCHTJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_OCL_OCLCHTJOIN_H_
#define SRC_JOIN_OCL_OCLCHTJOIN_H_

#include "../Join.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"

class CLEnv;
class CLProgram;

class OclCHTJoin: public Join {
protected:
	CLEnv* env;
	CLProgram* scanChtFull;

protected:
	Lookup* createLookup();
	const char* name();
public:
	OclCHTJoin(bool = false);
	virtual ~OclCHTJoin();
	virtual void join(kvlist* outer, kvlist* inner);
};

#endif /* SRC_JOIN_OCL_OCLCHTJOIN_H_ */
