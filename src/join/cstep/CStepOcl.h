/*
 * CstepOco.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_CSTEPOCL_H_
#define SRC_JOIN_CSTEP_CSTEPOCL_H_

#include "CStep.h"
#include "../../opencl/CLEnv.h"
#include "../../opencl/CLProgram.h"
#include "../../opencl/CLBuffer.h"

class CStepOcl: public CStep {
protected:
	uint meta[5];
	uint* cht_payload = NULL;
	uint* hash_payload = NULL;
	uint bitmapResultSize = 0;

	CLEnv* env = NULL;
	CLProgram* scanBitmap = NULL;
	CLProgram* scanCht = NULL;

	CLBuffer* metaBuffer = NULL;
	CLBuffer* bitmapBuffer = NULL;
	CLBuffer* chtpayloadBuffer = NULL;
	CLBuffer* hashpayloadBuffer = NULL;

public:
	CStepOcl();
	virtual ~CStepOcl();

	virtual void init();
	virtual uint filter(uint* gathered);
	virtual uint lookup(uint* key, uint keylength);
	virtual uint gather(ulong* bitmapResult, uint* gathered);

};

#endif /* SRC_JOIN_CSTEP_CSTEPOCL_H_ */
