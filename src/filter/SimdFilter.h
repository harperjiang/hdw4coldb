/*
 * SimdFilter.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_SIMDFILTER_H_
#define SRC_FILTER_SIMDFILTER_H_

#include "Filter.h"
#include "Pred.h"

class SimdFilter: public Filter {
protected:
	SimdPred* pred;
public:
	SimdFilter(OP, number);
	virtual ~SimdFilter();

	void filter(uint*, uint, uint*, uint*);
};

#endif /* SRC_FILTER_SIMDFILTER_H_ */
