/*
 * Filter.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_FILTER_H_
#define SRC_FILTER_FILTER_H_

#include <sys/types.h>

#include "Pred.h"

class Filter {
public:
	Filter();
	virtual ~Filter();

	virtual void filter(uint* input, uint inputSize, uint* output) = 0;
};

#endif /* SRC_FILTER_FILTER_H_ */
