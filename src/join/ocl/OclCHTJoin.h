/*
 * OclCHTJoin.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_OCL_OCLCHTJOIN_H_
#define SRC_JOIN_OCL_OCLCHTJOIN_H_

#include "../Join.h"

class OclCHTJoin: public Join {
public:
	OclCHTJoin();
	virtual ~OclCHTJoin();
	virtual void join(kvlist* outer, kvlist* inner, bool enableProfiling);
};

#endif /* SRC_JOIN_OCL_OCLCHTJOIN_H_ */
