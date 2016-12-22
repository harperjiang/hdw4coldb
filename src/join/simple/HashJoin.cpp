/*
 * HashJoin.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#include "../simple/HashJoin.h"

#include "../../lookup/Hash.h"

HashJoin::HashJoin() {
	// TODO Auto-generated constructor stub

}

HashJoin::~HashJoin() {
	// TODO Auto-generated destructor stub
}

Lookup* HashJoin::buildLookup(kvlist* outer) {
	Hash* hash = new Hash();
	hash->build(outer->entries, outer->size);
	return hash;
}
