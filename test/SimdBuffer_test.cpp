/*
 * SimdBuffer_test.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include <immintrin.h>
#include "../src/simd/SimdBuffer.h"

class SimdBufferTest: public SimdBuffer {
public:
	static __m256i testAlign(__m256i input, int* size) {
		return align(input, size);
	}
	static __m256i testShl(__m256i input, int offset) {
		return shl(input, offset);
	}
	static __m256i testShr(__m256i input, int offset) {
		return shr(input, offset);
	}
	static __m256i testMerge(__m256i a, __m256i b, int size) {
		return merge(a, b, size);
	}
};

TEST(SimdBuffer, Serve) {
	SimdBuffer* sbuf = new SimdBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi32(3, 0, 0, 2, 7, 1, 4, 8);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi32(0, 2, 0, 99, 0, 0, 241, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(8, outputSize);
	ASSERT_EQ(3, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 1));
	ASSERT_EQ(7, _mm256_extract_epi32(output, 2));
	ASSERT_EQ(1, _mm256_extract_epi32(output, 3));
	ASSERT_EQ(4, _mm256_extract_epi32(output, 4));
	ASSERT_EQ(8, _mm256_extract_epi32(output, 5));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 6));
	ASSERT_EQ(99, _mm256_extract_epi32(output, 7));

	input = _mm256_setr_epi32(0, 0, 0, 2324, 0, 0, 0, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	delete sbuf;
}

TEST(SimdBuffer, Purge) {
	SimdBuffer* sbuf = new SimdBuffer();

	int outputSize;
	__m256i input = _mm256_setr_epi32(3, 0, 0, 2, 7, 1, 4, 8);

	__m256i output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	input = _mm256_setr_epi32(0, 2, 0, 99, 0, 0, 241, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(8, outputSize);
	ASSERT_EQ(3, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 1));
	ASSERT_EQ(7, _mm256_extract_epi32(output, 2));
	ASSERT_EQ(1, _mm256_extract_epi32(output, 3));
	ASSERT_EQ(4, _mm256_extract_epi32(output, 4));
	ASSERT_EQ(8, _mm256_extract_epi32(output, 5));
	ASSERT_EQ(2, _mm256_extract_epi32(output, 6));
	ASSERT_EQ(99, _mm256_extract_epi32(output, 7));

	input = _mm256_setr_epi32(0, 0, 0, 2324, 0, 0, 0, 0);
	output = sbuf->serve(input, &outputSize);
	ASSERT_EQ(0, outputSize);

	output = sbuf->purge(&outputSize);
	ASSERT_EQ(2, outputSize);
	ASSERT_EQ(241, _mm256_extract_epi32(output, 0));
	ASSERT_EQ(2324, _mm256_extract_epi32(output, 1));

	delete sbuf;
}

TEST(SimdBuffer, Align) {
	// 10100110
	__m256i input = _mm256_setr_epi32(31331, 0, 22013, 0, 0, 124141, 551, 0);
	int size;
	__m256i result = SimdBufferTest::testAlign(input, &size);

	ASSERT_EQ(31331, _mm256_extract_epi32(result,0));
	ASSERT_EQ(22013, _mm256_extract_epi32(result,1));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,2));
	ASSERT_EQ(551, _mm256_extract_epi32(result,3));
	ASSERT_EQ(4, size);

	// 0000 1110
	input = _mm256_setr_epi32(0, 0, 0, 0, 5242152, 124141, 551, 0);
	result = SimdBufferTest::testAlign(input, &size);

	ASSERT_EQ(5242152, _mm256_extract_epi32(result,0));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,1));
	ASSERT_EQ(551, _mm256_extract_epi32(result,2));
	ASSERT_EQ(3, size);

	input = _mm256_setr_epi32(0, 0, 2719558, 0, 5242152, 124141, 551, 9895678);
	result = SimdBufferTest::testAlign(input, &size);

	ASSERT_EQ(2719558, _mm256_extract_epi32(result,0));
	ASSERT_EQ(5242152, _mm256_extract_epi32(result,1));
	ASSERT_EQ(124141, _mm256_extract_epi32(result,2));
	ASSERT_EQ(551, _mm256_extract_epi32(result,3));
	ASSERT_EQ(9895678, _mm256_extract_epi32(result,4));
	ASSERT_EQ(5, size);

	input = _mm256_setr_epi32(3, 0, 0, 2, 7, 1, 4, 8);
	result = SimdBufferTest::testAlign(input, &size);

	ASSERT_EQ(3, _mm256_extract_epi32(result,0));
	ASSERT_EQ(2, _mm256_extract_epi32(result,1));
	ASSERT_EQ(7, _mm256_extract_epi32(result,2));
	ASSERT_EQ(1, _mm256_extract_epi32(result,3));
	ASSERT_EQ(4, _mm256_extract_epi32(result,4));
	ASSERT_EQ(8, _mm256_extract_epi32(result,5));
	ASSERT_EQ(6, size);

	input = _mm256_setr_epi32(0, 2, 0, 99, 0, 0, 241, 0);
	result = SimdBufferTest::testAlign(input, &size);

	ASSERT_EQ(2, _mm256_extract_epi32(result,0));
	ASSERT_EQ(99, _mm256_extract_epi32(result,1));
	ASSERT_EQ(241, _mm256_extract_epi32(result,2));
	ASSERT_EQ(3, size);
}

TEST(SimdBuffer, Shl) {
	__m256i input = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
	__m256i s0 = SimdBufferTest::testShl(input, 0);
	ASSERT_EQ(1, _mm256_extract_epi32(s0,0));
	ASSERT_EQ(2, _mm256_extract_epi32(s0,1));
	ASSERT_EQ(3, _mm256_extract_epi32(s0,2));
	ASSERT_EQ(4, _mm256_extract_epi32(s0,3));
	ASSERT_EQ(5, _mm256_extract_epi32(s0,4));
	ASSERT_EQ(6, _mm256_extract_epi32(s0,5));
	ASSERT_EQ(7, _mm256_extract_epi32(s0,6));
	ASSERT_EQ(8, _mm256_extract_epi32(s0,7));

	__m256i s1 = SimdBufferTest::testShl(input, 1);
	ASSERT_EQ(2, _mm256_extract_epi32(s1,0));
	ASSERT_EQ(3, _mm256_extract_epi32(s1,1));
	ASSERT_EQ(4, _mm256_extract_epi32(s1,2));
	ASSERT_EQ(5, _mm256_extract_epi32(s1,3));
	ASSERT_EQ(6, _mm256_extract_epi32(s1,4));
	ASSERT_EQ(7, _mm256_extract_epi32(s1,5));
	ASSERT_EQ(8, _mm256_extract_epi32(s1,6));
	ASSERT_EQ(1, _mm256_extract_epi32(s1,7));

	__m256i s2 = SimdBufferTest::testShl(input, 2);
	ASSERT_EQ(3, _mm256_extract_epi32(s2,0));
	ASSERT_EQ(4, _mm256_extract_epi32(s2,1));
	ASSERT_EQ(5, _mm256_extract_epi32(s2,2));
	ASSERT_EQ(6, _mm256_extract_epi32(s2,3));
	ASSERT_EQ(7, _mm256_extract_epi32(s2,4));
	ASSERT_EQ(8, _mm256_extract_epi32(s2,5));
	ASSERT_EQ(1, _mm256_extract_epi32(s2,6));
	ASSERT_EQ(2, _mm256_extract_epi32(s2,7));

	__m256i s3 = SimdBufferTest::testShl(input, 3);
	ASSERT_EQ(4, _mm256_extract_epi32(s3,0));
	ASSERT_EQ(5, _mm256_extract_epi32(s3,1));
	ASSERT_EQ(6, _mm256_extract_epi32(s3,2));
	ASSERT_EQ(7, _mm256_extract_epi32(s3,3));
	ASSERT_EQ(8, _mm256_extract_epi32(s3,4));
	ASSERT_EQ(1, _mm256_extract_epi32(s3,5));
	ASSERT_EQ(2, _mm256_extract_epi32(s3,6));
	ASSERT_EQ(3, _mm256_extract_epi32(s3,7));

	__m256i s4 = SimdBufferTest::testShl(input, 4);
	ASSERT_EQ(5, _mm256_extract_epi32(s4,0));
	ASSERT_EQ(6, _mm256_extract_epi32(s4,1));
	ASSERT_EQ(7, _mm256_extract_epi32(s4,2));
	ASSERT_EQ(8, _mm256_extract_epi32(s4,3));
	ASSERT_EQ(1, _mm256_extract_epi32(s4,4));
	ASSERT_EQ(2, _mm256_extract_epi32(s4,5));
	ASSERT_EQ(3, _mm256_extract_epi32(s4,6));
	ASSERT_EQ(4, _mm256_extract_epi32(s4,7));

	__m256i s5 = SimdBufferTest::testShl(input, 5);
	ASSERT_EQ(6, _mm256_extract_epi32(s5,0));
	ASSERT_EQ(7, _mm256_extract_epi32(s5,1));
	ASSERT_EQ(8, _mm256_extract_epi32(s5,2));
	ASSERT_EQ(1, _mm256_extract_epi32(s5,3));
	ASSERT_EQ(2, _mm256_extract_epi32(s5,4));
	ASSERT_EQ(3, _mm256_extract_epi32(s5,5));
	ASSERT_EQ(4, _mm256_extract_epi32(s5,6));
	ASSERT_EQ(5, _mm256_extract_epi32(s5,7));

	__m256i s6 = SimdBufferTest::testShl(input, 6);
	ASSERT_EQ(7, _mm256_extract_epi32(s6,0));
	ASSERT_EQ(8, _mm256_extract_epi32(s6,1));
	ASSERT_EQ(1, _mm256_extract_epi32(s6,2));
	ASSERT_EQ(2, _mm256_extract_epi32(s6,3));
	ASSERT_EQ(3, _mm256_extract_epi32(s6,4));
	ASSERT_EQ(4, _mm256_extract_epi32(s6,5));
	ASSERT_EQ(5, _mm256_extract_epi32(s6,6));
	ASSERT_EQ(6, _mm256_extract_epi32(s6,7));

	__m256i s7 = SimdBufferTest::testShl(input, 7);
	ASSERT_EQ(8, _mm256_extract_epi32(s7,0));
	ASSERT_EQ(1, _mm256_extract_epi32(s7,1));
	ASSERT_EQ(2, _mm256_extract_epi32(s7,2));
	ASSERT_EQ(3, _mm256_extract_epi32(s7,3));
	ASSERT_EQ(4, _mm256_extract_epi32(s7,4));
	ASSERT_EQ(5, _mm256_extract_epi32(s7,5));
	ASSERT_EQ(6, _mm256_extract_epi32(s7,6));
	ASSERT_EQ(7, _mm256_extract_epi32(s7,7));
}

TEST(SimdBuffer, Shr) {
	__m256i input = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
	__m256i s0 = SimdBufferTest::testShr(input, 0);
	ASSERT_EQ(1, _mm256_extract_epi32(s0,0));
	ASSERT_EQ(2, _mm256_extract_epi32(s0,1));
	ASSERT_EQ(3, _mm256_extract_epi32(s0,2));
	ASSERT_EQ(4, _mm256_extract_epi32(s0,3));
	ASSERT_EQ(5, _mm256_extract_epi32(s0,4));
	ASSERT_EQ(6, _mm256_extract_epi32(s0,5));
	ASSERT_EQ(7, _mm256_extract_epi32(s0,6));
	ASSERT_EQ(8, _mm256_extract_epi32(s0,7));

	__m256i s1 = SimdBufferTest::testShr(input, 1);
	ASSERT_EQ(8, _mm256_extract_epi32(s1,0));
	ASSERT_EQ(1, _mm256_extract_epi32(s1,1));
	ASSERT_EQ(2, _mm256_extract_epi32(s1,2));
	ASSERT_EQ(3, _mm256_extract_epi32(s1,3));
	ASSERT_EQ(4, _mm256_extract_epi32(s1,4));
	ASSERT_EQ(5, _mm256_extract_epi32(s1,5));
	ASSERT_EQ(6, _mm256_extract_epi32(s1,6));
	ASSERT_EQ(7, _mm256_extract_epi32(s1,7));

	__m256i s2 = SimdBufferTest::testShr(input, 2);
	ASSERT_EQ(7, _mm256_extract_epi32(s2,0));
	ASSERT_EQ(8, _mm256_extract_epi32(s2,1));
	ASSERT_EQ(1, _mm256_extract_epi32(s2,2));
	ASSERT_EQ(2, _mm256_extract_epi32(s2,3));
	ASSERT_EQ(3, _mm256_extract_epi32(s2,4));
	ASSERT_EQ(4, _mm256_extract_epi32(s2,5));
	ASSERT_EQ(5, _mm256_extract_epi32(s2,6));
	ASSERT_EQ(6, _mm256_extract_epi32(s2,7));

	__m256i s3 = SimdBufferTest::testShr(input, 3);
	ASSERT_EQ(6, _mm256_extract_epi32(s3,0));
	ASSERT_EQ(7, _mm256_extract_epi32(s3,1));
	ASSERT_EQ(8, _mm256_extract_epi32(s3,2));
	ASSERT_EQ(1, _mm256_extract_epi32(s3,3));
	ASSERT_EQ(2, _mm256_extract_epi32(s3,4));
	ASSERT_EQ(3, _mm256_extract_epi32(s3,5));
	ASSERT_EQ(4, _mm256_extract_epi32(s3,6));
	ASSERT_EQ(5, _mm256_extract_epi32(s3,7));

	__m256i s4 = SimdBufferTest::testShr(input, 4);
	ASSERT_EQ(5, _mm256_extract_epi32(s4,0));
	ASSERT_EQ(6, _mm256_extract_epi32(s4,1));
	ASSERT_EQ(7, _mm256_extract_epi32(s4,2));
	ASSERT_EQ(8, _mm256_extract_epi32(s4,3));
	ASSERT_EQ(1, _mm256_extract_epi32(s4,4));
	ASSERT_EQ(2, _mm256_extract_epi32(s4,5));
	ASSERT_EQ(3, _mm256_extract_epi32(s4,6));
	ASSERT_EQ(4, _mm256_extract_epi32(s4,7));

	__m256i s5 = SimdBufferTest::testShr(input, 5);
	ASSERT_EQ(4, _mm256_extract_epi32(s5,0));
	ASSERT_EQ(5, _mm256_extract_epi32(s5,1));
	ASSERT_EQ(6, _mm256_extract_epi32(s5,2));
	ASSERT_EQ(7, _mm256_extract_epi32(s5,3));
	ASSERT_EQ(8, _mm256_extract_epi32(s5,4));
	ASSERT_EQ(1, _mm256_extract_epi32(s5,5));
	ASSERT_EQ(2, _mm256_extract_epi32(s5,6));
	ASSERT_EQ(3, _mm256_extract_epi32(s5,7));

	__m256i s6 = SimdBufferTest::testShr(input, 6);
	ASSERT_EQ(3, _mm256_extract_epi32(s6,0));
	ASSERT_EQ(4, _mm256_extract_epi32(s6,1));
	ASSERT_EQ(5, _mm256_extract_epi32(s6,2));
	ASSERT_EQ(6, _mm256_extract_epi32(s6,3));
	ASSERT_EQ(7, _mm256_extract_epi32(s6,4));
	ASSERT_EQ(8, _mm256_extract_epi32(s6,5));
	ASSERT_EQ(1, _mm256_extract_epi32(s6,6));
	ASSERT_EQ(2, _mm256_extract_epi32(s6,7));

	__m256i s7 = SimdBufferTest::testShr(input, 7);
	ASSERT_EQ(2, _mm256_extract_epi32(s7,0));
	ASSERT_EQ(3, _mm256_extract_epi32(s7,1));
	ASSERT_EQ(4, _mm256_extract_epi32(s7,2));
	ASSERT_EQ(5, _mm256_extract_epi32(s7,3));
	ASSERT_EQ(6, _mm256_extract_epi32(s7,4));
	ASSERT_EQ(7, _mm256_extract_epi32(s7,5));
	ASSERT_EQ(8, _mm256_extract_epi32(s7,6));
	ASSERT_EQ(1, _mm256_extract_epi32(s7,7));
}

TEST(SimdBuffer, Merge) {
	__m256i a = _mm256_setr_epi32(2, 4, 6, 8, 10, 12, 14, 16);
	__m256i b = _mm256_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15);

	__m256i merge = SimdBufferTest::testMerge(a, b, 0);
	ASSERT_EQ(1, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(13, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(15, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 1);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(13, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 2);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(11, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 3);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(9, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 4);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(7, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 5);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(5, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 6);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(12, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(3, _mm256_extract_epi32(merge,7));

	merge = SimdBufferTest::testMerge(a, b, 7);
	ASSERT_EQ(2, _mm256_extract_epi32(merge,0));
	ASSERT_EQ(4, _mm256_extract_epi32(merge,1));
	ASSERT_EQ(6, _mm256_extract_epi32(merge,2));
	ASSERT_EQ(8, _mm256_extract_epi32(merge,3));
	ASSERT_EQ(10, _mm256_extract_epi32(merge,4));
	ASSERT_EQ(12, _mm256_extract_epi32(merge,5));
	ASSERT_EQ(14, _mm256_extract_epi32(merge,6));
	ASSERT_EQ(1, _mm256_extract_epi32(merge,7));
}
