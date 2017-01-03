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

inline uint64_t popcount64(uint64_t data);

inline uint64_t popcount64(uint64_t input) {
	uint64_t result = 0;
	asm("popcnt %1, %0": "=r"(result) : "rm"(input):"cc");
	return result;
}

/*
 * Multiplicative hashing
 */
inline uint32_t mut_hash(uint32_t v);

inline uint32_t mut_hash(uint32_t v) {
	return v * UINT32_C(2654435761);
}

inline uint32_t mut_hash2(uint32_t v);

inline uint32_t mut_hash2(uint32_t v) {
	return v * UINT32_C(3729515385);
}

inline uint32_t num_roundup(uint32_t n);

inline uint32_t num_roundup(uint32_t n) {
	n |= (n >> 1);
	n |= (n >> 2);
	n |= (n >> 4);
	n |= (n >> 8);
	n |= (n >> 16);
	return (n == 0xffffffff) ? n : n + 1;
}

/**
 * Load key from files
 */
void loadkey(const char* filename, kvlist* result);

#endif /* SRC_UTIL_H_ */
