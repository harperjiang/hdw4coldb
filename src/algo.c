/*
 * algo.c
 *
 *  Created on: Nov 23, 2016
 *      Author: Cathy
 */

#include "algo.h"

extern void hash_init_class();
extern void cht_init_class();

void init_class() {
	hash_init_class();
	cht_init_class();
}
