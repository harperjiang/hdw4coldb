/*
 * SIMDFilter.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_SIMDFILTER_H_
#define SRC_FILTER_SIMDFILTER_H_

#include "Filter.h"

class SIMDFilter: public Filter {
public:
	SIMDFilter(OP, uint);
	virtual ~SIMDFilter();

	void filter(uint*, uint, uint*);
};

#endif /* SRC_FILTER_SIMDFILTER_H_ */
