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

union number {
	uint integer;
	float floatnum;
};

class ScalarPred {
protected:
	number target;
public:
	ScalarPred(number target);
	virtual ~ScalarPred();

	virtual bool testInteger(uint input) = 0;
	virtual bool testFloat(float input) = 0;

	static ScalarPred* create(OP, number);
};

class ScalarEqual: public ScalarPred {
public:
	ScalarEqual(number target);
	virtual ~ScalarEqual();

	bool testInteger(uint input);
	bool testFloat(float input);
};

class SimdPred {
protected:
	__m256i targetint;
	__m256 targetfloat;
public:
	SimdPred(number target);
	virtual ~SimdPred();

	void* operator new(size_t sz);

	virtual __m256i testInteger(__m256i input) = 0;
	virtual __m256 testFloat(__m256 input) = 0;

	static SimdPred* create(OP, number);
};

class SimdEqual: public SimdPred {
public:
	SimdEqual(number target);
	virtual ~SimdEqual();

	__m256i testInteger(__m256i input);
	__m256 testFloat(__m256 input);
};

#endif /* SRC_FILTER_PRED_H_ */
