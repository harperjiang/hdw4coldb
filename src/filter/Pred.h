/*
 * Pred.h
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#ifndef SRC_FILTER_PRED_H_
#define SRC_FILTER_PRED_H_

#include <sys/types.h>
#include <immintrin.h>

enum OP {
	EQ, NEQ, GT, LT, GE, LE
};

class ScalarPred {
protected:
	uint target;
public:
	ScalarPred(uint target);
	virtual ~ScalarPred();

	virtual bool testInteger(uint input) = 0;
	virtual bool testFloat(float input) = 0;

	static ScalarPred create(OP, uint);
};

class ScalarEqual : public ScalarPred {
public:
	ScalarEqual(uint target);
	virtual ~ScalarEqual();

	bool testInteger(uint input);
	bool testFloat(float input);
};

class SIMDPred {
protected:
	uint target;
public:
	SIMDPred(uint target);
	virtual ~SIMDPred();

	virtual __m256i testInteger(__m256i input) = 0;
	virtual __m256 testFloat(__m256 input) = 0;

	static SIMDPred create(OP,uint);
};

class SIMDEqual: public SIMDPred {
public:
	SIMDEqual(uint target);
	virtual ~SIMDEqual();

	__m256i testInteger(__m256i input);
	__m256 testFloat(__m256 input);
};

#endif /* SRC_FILTER_PRED_H_ */
