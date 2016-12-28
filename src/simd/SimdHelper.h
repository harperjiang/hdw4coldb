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
	virtual __m256i transform(__m256i);
	virtual __m256i transform2(__m256i, __m256i);
	virtual __m256i transform3(__m256i, __m256i *);
};

class SimdHelper {
public:
	SimdHelper();
	virtual ~SimdHelper();

	static __m256i ZERO;
	static __m256i ONE;
	static __m256i TWO;
	static __m256i FOUR;
	static __m256i MAX;
	static __m256i THIRTY_ONE;
	static __m256i TWENTY_FOUR;
protected:
	static __m256i POPCNT_WWG_C1;
	static __m256i POPCNT_WWG_C2;
	static __m256i POPCNT_WWG_C3;
	static __m256i POPCNT_WWG_C4;

	static __m256i POPCNT_MULA_C;
protected:
	/**
	 * Popcount use WWG algorithm
	 */
	static __m256i popcnt_epi32_wwg(__m256i input);
	/**
	 * Popcount use Mula's Algorithm
	 */
	static __m256i popcnt_epi32_mula(__m256i input);
public:
	static void transform(uint* src, uint srclength, uint* dest,
			SimdTransform* trans, bool enableProfiling = false);
	static void transform2(uint* srca, uint* srcb, uint srclength, uint* dest,
			SimdTransform* trans, bool enableProfiling = false);
	static void transform3(uint* src, uint srclength, uint* dest1, uint* dest2,
			SimdTransform* trans, bool enableProfiling = false);

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
