/*
 * util.h
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <stdint.h>

/**
 * Popcount
 */
uint32_t popcount(uint32_t data);

/*
 * Multiplicative hashing
 */
uint32_t hash(uint32_t v);

#endif /* SRC_UTIL_H_ */
