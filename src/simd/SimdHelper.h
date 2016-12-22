/*
 * SimdHelper.h
 *
 *  Created on: Dec 22, 2016
 *      Author: Cathy
 */

#ifndef SRC_SIMD_SIMDHELPER_H_
#define SRC_SIMD_SIMDHELPER_H_

#include <stdlib.h>
#include <sys/types.h>
#include <immintrin.h>

class SimdTransform {
public:
	SimdTransform() {
	}
	virtual ~SimdTransform() {
	}
	virtual __m256i transform(__m256i) = 0;
};

class SimdHelper {
public:
	SimdHelper();
	virtual ~SimdHelper();

	static void transform(uint* src, uint srclength, uint* dest,
			SimdTransform* trans);
	static void print_epu32(__m256i a);
	static __m256i remainder_epu32(__m256i a, uint b);
	static __m256i divrem_epu32(__m256i* remainder, __m256i a, uint b);
	static __m256i popcnt_epi32(__m256i input);
	/*
	 * set -1 for 0, 0 for nz
	 */
	static __m256i testz_epi32(__m256i input);
	/*
	 * set -1 for nz, 0 for zero
	 */
	static __m256i testnz_epi32(__m256i input);
	static void store_epu32(uint* base, uint offset, __m256i input, uint length);
};

#endif /* SRC_SIMD_SIMDHELPER_H_ */
