/*
 * Pred.cpp
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#include <stdlib.h>
#include "Pred.h"

ScalarPred::ScalarPred(number target) {
	this->target = target;
}

ScalarPred::~ScalarPred() {

}

ScalarPred* ScalarPred::create(OP op, number target) {
	switch (op) {
	case EQ:
		return new ScalarEqual(target);
	default:
		return NULL;
	}
}

ScalarEqual::ScalarEqual(number target) :
		ScalarPred(target) {

}

ScalarEqual::~ScalarEqual() {

}

bool ScalarEqual::testInteger(uint input) {
	return input == target.integer;
}

bool ScalarEqual::testFloat(float input) {
	return input == target.floatnum;
}

SimdPred::SimdPred(number target) {
	this->targetint = _mm256_set1_epi32(target.integer);
	this->targetfloat = _mm256_set1_ps(target.floatnum);
}

SimdPred::~SimdPred() {

}

void* SimdPred::operator new(size_t sz) {
	return aligned_alloc(32, sz);
}

SimdPred* SimdPred::create(OP op, number target) {
	switch (op) {
	case EQ:
		return new SimdEqual(target);
	default:
		return NULL;
	}
}

SimdEqual::SimdEqual(number target) :
		SimdPred(target) {

}

SimdEqual::~SimdEqual() {

}

__m256i SimdEqual::testInteger(__m256i input) {
	return _mm256_and_si256(input, _mm256_cmpeq_epi32(input, targetint));
}

__m256 SimdEqual::testFloat(__m256 input) {
	return _mm256_and_ps(input, _mm256_cmp_ps(input, targetfloat, 0));
}
