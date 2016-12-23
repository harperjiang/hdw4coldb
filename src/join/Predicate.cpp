/*
 * Predicate.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#include "Predicate.h"

Predicate::Predicate() {
	// TODO Auto-generated constructor stub

}

Predicate::~Predicate() {
	// TODO Auto-generated destructor stub
}

Equal::Equal(uint val) {
	this->value = val;
}

Equal::~Equal() {

}

bool Equal::test(uint input) {
	return input == this->value;
}

NotEqual::NotEqual(uint val) {
	this->value = val;
}

NotEqual::~NotEqual() {

}

bool NotEqual::test(uint input) {
	return input != this->value;
}
