/*
 * CHTJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "CHTJoin.h"
#include "../../lookup/CHT.h"

CHTJoin::CHTJoin() {

}

CHTJoin::~CHTJoin() {

}

Lookup* CHTJoin::buildLookup(kvlist* outer) {
	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	return cht;
}
