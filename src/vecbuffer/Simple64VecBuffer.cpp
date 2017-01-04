/*
 * Simple64VecBuffer.cpp
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#include "Simple64VecBuffer.h"

Simple64VecBuffer::Simple64VecBuffer() {

}

Simple64VecBuffer::~Simple64VecBuffer() {
}

__m256i Simple64VecBuffer::serve(__m256i input, int* outputSize) {
	ulong data[4];
	uint size = 0;
	data[0] = _mm256_extract_epi64(input, 0);
	size += data[0] ? 1 : 0;
	data[1] = _mm256_extract_epi64(input, 0);
	size += data[1] ? 1 : 0;
	data[2] = _mm256_extract_epi64(input, 0);
	size += data[2] ? 1 : 0;
	data[3] = _mm256_extract_epi64(input, 0);
	size += data[3] ? 1 : 0;

	__m256i result;
	if (bufferSize + size >= 4) {
		*outputSize = 4;
		for (int i = 0; i < 4; i++) {
			if (bufferSize >= 4) {
				result = _mm256_setr_epi64x(buffer[0], buffer[1], buffer[2],
						buffer[3]);
				bufferSize = 0;
			}
			if (data[i] != 0) {
				buffer[bufferSize++] = data[i];
			}
		}
	} else {
		*outputSize = 0;
		for (int i = 0; i < 4; i++) {
			if (data[i] != 0) {
				buffer[bufferSize++] = data[i];
			}
		}
		result = _mm256_setzero_si256();
	}
	return result;
}

__m256i Simple64VecBuffer::purge(int* outputSize) {
	*outputSize = bufferSize;
	bufferSize = 0;
	return _mm256_setr_epi64x(buffer[0], buffer[1], buffer[2], buffer[3]);
}
