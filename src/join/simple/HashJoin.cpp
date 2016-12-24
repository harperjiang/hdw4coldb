/*
 * HashJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "../simple/HashJoin.h"

#include "../../lookup/Hash.h"

HashJoin::HashJoin(uint numThread, bool ep) :
		LookupJoin(numThread, ep) {

}

HashJoin::~HashJoin() {
}

Lookup* HashJoin::createLookup() {
	return new Hash();
}

const char* HashJoin::name() {
	return "HashJoin";
}
