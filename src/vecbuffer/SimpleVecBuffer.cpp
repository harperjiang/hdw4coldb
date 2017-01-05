/*
 * SimpleVecBuffer.cpp
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#include "SimpleVecBuffer.h"

SimpleVecBuffer::SimpleVecBuffer() {
	bufferSize = 0;

}

SimpleVecBuffer::~SimpleVecBuffer() {

}

__m256i SimpleVecBuffer::serve(__m256i input, int* outputSize) {
	int data[8];
	int osize = 0;
	data[0] = _mm256_extract_epi32(input, 0);
	osize += data[0] ? 1 : 0;
	data[1] = _mm256_extract_epi32(input, 1);
	osize += data[1] ? 1 : 0;
	data[2] = _mm256_extract_epi32(input, 2);
	osize += data[2] ? 1 : 0;
	data[3] = _mm256_extract_epi32(input, 3);
	osize += data[3] ? 1 : 0;
	data[4] = _mm256_extract_epi32(input, 4);
	osize += data[4] ? 1 : 0;
	data[5] = _mm256_extract_epi32(input, 5);
	osize += data[5] ? 1 : 0;
	data[6] = _mm256_extract_epi32(input, 6);
	osize += data[6] ? 1 : 0;
	data[7] = _mm256_extract_epi32(input, 7);
	osize += data[7] ? 1 : 0;

	__m256i result;
	if (osize + bufferSize >= 8) {
		// Fill buffer
		int pointer = 0;
		for (pointer = 0; pointer < 8; pointer++) {
			if (bufferSize >= 8) {
				result = _mm256_setr_epi32(buffer[0], buffer[1], buffer[2],
						buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
				bufferSize = 0;
			}
			if (data[pointer] != 0) {
				buffer[bufferSize++] = data[pointer];
			}
		}

		*outputSize = 8;
		return result;
	} else {
		*outputSize = 0;

		// Copy buffer
		for (int i = 0; i < 8; i++) {
			if (data[i] != 0) {
				buffer[bufferSize++] = data[i];
			}
		}

		return _mm256_setzero_si256();
	}
}

__m256i SimpleVecBuffer::purge(int* outputSize) {
	*outputSize = bufferSize;
	bufferSize = 0;
	return _mm256_setr_epi32(buffer[0], buffer[1], buffer[2], buffer[3],
			buffer[4], buffer[5], buffer[6], buffer[7]);
}
