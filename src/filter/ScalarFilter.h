/*
 * ScalarFilter.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_SCALARFILTER_H_
#define SRC_FILTER_SCALARFILTER_H_

#include "Filter.h"
#include "Pred.h"

class ScalarFilter: public Filter {
protected:
	ScalarPred* pred;
public:
	ScalarFilter(OP, number);
	virtual ~ScalarFilter();

	void filter(uint* input, uint inputSize, uint* output, uint* outputSize);
};

#endif /* SRC_FILTER_SCALARFILTER_H_ */
