/*
 * Join.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "Join.h"
#include "../util/Logger.h"

Join::Join() {
	_logger = NULL;
	_matched = new Matched();
}

Join::~Join() {
}

void Join::setMatched(Matched* m) {
	this->_matched = m;
}

Matched* Join::getMatched() {
	return this->_matched;
}
