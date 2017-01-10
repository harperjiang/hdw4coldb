/*
 * ScalarFilter.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_SCALARFILTER_H_
#define SRC_FILTER_SCALARFILTER_H_

#include "Filter.h"

class ScalarFilter: public Filter {
public:
	ScalarFilter(Pred*);
	virtual ~ScalarFilter();

	void filter(uint* input, uint inputSize, uint* output);
};

#endif /* SRC_FILTER_SCALARFILTER_H_ */
