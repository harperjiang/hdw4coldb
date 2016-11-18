/*
 * util.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "util.h"

uint32_t popcount(uint32_t input) {
	uint32_t result = 0;
	if (__builtin_cpu_supports("popcnt")) {
		asm("popcnt %1, %0": "=r"(result) : "rm"(input):"cc");
	} else {
		// Simple implementation, just in case
		uint32_t mask = 1;
		for (int i = 0; i < 32; i++) {
			result += ((mask << i) & input) ? 1 : 0;
		}
	}
	return result;
}

uint32_t hash(uint32_t v) {
	return v * UINT32_C(2654435761);
}
