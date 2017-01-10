/*
 * Pred.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_PRED_H_
#define SRC_FILTER_PRED_H_

#include <sys/types.h>

enum OP {
	EQ, NEQ, GT, LT, GE, LE
};

class Pred {
private:
	OP op;
	uint target;
public:
	Pred(OP, uint);
	virtual ~Pred();

	OP getOP();
	uint getInteger();
	double getDouble();
};

#endif /* SRC_FILTER_PRED_H_ */
