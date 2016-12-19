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
private:
	uint meta[5];
	uint* cht_payload;
	uint* hash_payload;

	CLEnv* env;
	CLProgram* scanBitmap;
	CLProgram* scanCht;

	CLBuffer* metaBuffer = NULL;
	CLBuffer* bitmapBuffer = NULL;
	CLBuffer* chtpayloadBuffer = NULL;
	CLBuffer* hashpayloadBuffer = NULL;

public:
	CStepOcl();
	virtual ~CStepOcl();

	void init(CHT* lookup, uint* key, uint keylength);

	virtual uint filter(uint* key, uint keylength, ulong* bitmap,
			uint bitmapSize, uint* gathered);
	uint gather(ulong* resultBitmap, uint resultBitmapSize, uint* key,
			uint keylength, uint* gathered);
	uint lookup(CHT* lookup, uint* key, uint keylength);

};

#endif /* SRC_JOIN_CSTEP_CSTEPOCL_H_ */
