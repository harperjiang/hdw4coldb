/*
 * Pred.cpp
 *
 *  Created on: Jan 10, 2017
 *      Author: harper
 */

#include "Pred.h"

Pred::Pred(OP op, uint target) {
	this->op = op;
	this->target = target;
}

Pred::~Pred() {

}

OP Pred::getOP() {
	return op;
}

uint Pred::getInteger() {
	return target;
}

double Pred::getDouble() {
	return (double) target;
}

