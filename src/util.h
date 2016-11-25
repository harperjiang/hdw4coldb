/*
 * util.h
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Lookup.h"
/**
 * Popcount
 */
inline uint32_t popcount(uint32_t data);

inline uint32_t popcount(uint32_t input) {
	uint32_t result = 0;
	//if (__builtin_cpu_supports("popcnt")) {
	asm("popcnt %1, %0": "=r"(result) : "rm"(input):"cc");
	/*} else {
	 // Simple implementation, just in case
	 uint32_t mask = 1;
	 for (int i = 0; i < 32; i++) {
	 result += ((mask << i) & input) ? 1 : 0;
	 }
	 }*/
	return result;
}
/*
 * Multiplicative hashing
 */
inline uint32_t hash(uint32_t v);

inline uint32_t hash(uint32_t v) {
	return v * UINT32_C(2654435761);
}

/**
 * Load key from files
 */
void loadkey(const char* filename, kvlist* result);

#endif /* SRC_UTIL_H_ */
