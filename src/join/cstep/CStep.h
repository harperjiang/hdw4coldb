/*
 * CStep.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_CSTEP_CSTEP_H_
#define SRC_JOIN_CSTEP_CSTEP_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../lookup/Lookup.h"
#include "../../lookup/CHT.h"

#define THRESHOLD 5
#define BITMAP_SIZE		32
#define BITMAP_EXT 		32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define RET_BITMAP_UNIT 	64

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

class CStep {

protected:
	bool enableProfiling;
public:
	CStep();
	virtual ~CStep();

	virtual void join(kvlist* outer, kvlist* inner, uint split,
			bool enableProfiling);
	virtual void init(CHT* lookup, uint* key, uint keylength) = 0;

	virtual uint filter(uint* key, uint keylength, ulong* bitmap,
			uint bitmapSize, uint* gathered) = 0;

	virtual uint lookup(CHT* lookup, uint* key, uint keylength) = 0;
};

#endif /* SRC_JOIN_CSTEP_CSTEP_H_ */
