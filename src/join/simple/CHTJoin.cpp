/*
 * CHTJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "CHTJoin.h"
#include "../../lookup/CHT.h"

CHTJoin::CHTJoin(bool ep) :
		LookupJoin(ep) {

}

CHTJoin::~CHTJoin() {

}

Lookup* CHTJoin::createLookup() {
	return new CHT();
}
