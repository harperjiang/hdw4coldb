/*
 * CHTJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "CHTJoin.h"
#include "../../lookup/CHT.h"

CHTJoin::CHTJoin(uint numThread, bool ep) :
		LookupJoin(numThread, ep) {

}

CHTJoin::~CHTJoin() {

}

Lookup* CHTJoin::createLookup() {
	return new CHT();
}

const char* CHTJoin::name() {
	return "CHTJoin";
}
