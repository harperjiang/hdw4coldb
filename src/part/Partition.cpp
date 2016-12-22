/*
 * Partition.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: Cathy
 */

#include "Partition.h"

Partition::Partition() {

}

Partition::~Partition() {

}

kvlist* Partition::getInner() {
	return inner;
}

kvlist* Partition::getOuter() {
	return outer;
}

Partitioner::Partitioner() {

}

Partitioner::~Partitioner() {

}
