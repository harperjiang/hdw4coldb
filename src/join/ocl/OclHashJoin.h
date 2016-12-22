/*
 * OclHashJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_OCL_OCLHASHJOIN_H_
#define SRC_JOIN_OCL_OCLHASHJOIN_H_

#include "../Join.h"

class OclHashJoin: public Join {
public:
	OclHashJoin();
	virtual ~OclHashJoin();
	virtual void join(kvlist* outer, kvlist* inner, uint split,
			bool enableProfiling);
};

#endif /* SRC_JOIN_OCL_OCLHASHJOIN_H_ */
