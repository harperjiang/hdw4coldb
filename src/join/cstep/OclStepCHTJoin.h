/*
 * OclStepCHTJoin.h
 *
 *  Created on: Dec 23, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_CSTEP_OCLSTEPCHTJOIN_H_
#define SRC_JOIN_CSTEP_OCLSTEPCHTJOIN_H_

#include "StepCHTJoin.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"
#include "../../opencl/CLBuffer.h"

class OclStepCHTJoin: public StepCHTJoin {
public:
	OclStepCHTJoin(bool = false, bool = false, bool = false);
	virtual ~OclStepCHTJoin();

protected:
	CLEnv* env = NULL;
	CLProgram* _scanBitmap = NULL;
	CLProgram* _scanCht = NULL;

	uint meta[5];
	CLBuffer* metaBuffer = NULL;

	uint bitmapResultSize = 0;
	CLBuffer* bitmapBuffer = NULL;
protected:
	void init();
	void filter();
	void scanCht();
	void scanHash();
	void collect();

	const char* name();
};

#endif /* SRC_JOIN_CSTEP_OCLSTEPCHTJOIN_H_ */
