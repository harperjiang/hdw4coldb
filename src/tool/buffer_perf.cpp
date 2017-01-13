/*
 * buffer_perf.cpp
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <random>
#include <getopt.h>
#include <time.h>
#include "../util/Logger.h"
#include "../util/Timer.h"
#include "../simd/SimdHelper.h"
#include "../vecbuffer/VecBuffer.h"
#include "../vecbuffer/SimdVecBuffer.h"
#include "../vecbuffer/MTableVecBuffer.h"
#include "../vecbuffer/SimpleVecBuffer.h"
#include "../vecbuffer/Simd64VecBuffer.h"
#include "../vecbuffer/Simple64VecBuffer.h"

uint* gendata(double portion, uint size) {
	srand(time(NULL));
	uint* data = (uint*) aligned_alloc(32, size * sizeof(uint));
	uint thres = (uint) UINT32_C(0xFFFFFFFF) * portion;
	for (uint i = 0; i < size; i++) {
		uint val = (uint) rand();
		data[i] = val < thres ? val : 0;
	}
	return data;
}

__m256i genrand(double portion) {
	uint thres = (uint) (UINT32_C(0xFFFFFFFF) * portion);
	int data[8];
	for (uint i = 0; i < 8; i++) {
		uint val = (uint) rand();
		data[i] = val < thres ? val : 0;
	}
	return _mm256_setr_epi32(data[0], data[1], data[2], data[3], data[4],
			data[5], data[6], data[7]);
}

void run(uint size, double portion, VecBuffer* buffer) {
	srand(time(NULL));
	int outputSize;

	Logger* logger = Logger::getLogger("perf_buffer");
	Timer timer;

	timer.start();
	__m256i logging = _mm256_setzero_si256();
	for (int i = 0; i < size; i++) {
		__m256i input = genrand(portion);
		__m256i result = buffer->serve(input, &outputSize);
		logging = _mm256_and_si256(logging, result);
	}
	__m256i final = buffer->purge(&outputSize);

	logging = _mm256_and_si256(logging, final);

	timer.stop();

	logger->info("Running size %u, time consumption %lu\n", size,
			timer.wallclockms());
	SimdHelper::print_epu32(logging);
}

void print_help() {
	fprintf(stdout, "Usage: buffer_perf [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg alg_name \tAlgorithm to use\n");
	fprintf(stdout, " -p --portion \tportion of non-zeros\n");
	fprintf(stdout, " -s --size \tsize of test\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	exit(0);
}

int main(int argc, char** argv) {
	Logger* logger = Logger::getLogger("buffer_perf");

	bool useBuffer = false;
	double portion = 0.5;
	uint size = 1000000;
	char* alg;

	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "portion",
			required_argument, 0, 'p' }, { "help", no_argument, 0, 'h' }, {
					"size", required_argument, 0, 's' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:p:hs:", long_options, &option_index))
			!= -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'p':
			portion = atof(optarg);
			break;
		case 'h':
			print_help();
			break;
		case 's':
			size = atoi(optarg);
			break;
		default:
			fprintf(stderr, "unrecognized option or missing argument\n");
			print_help();
			break;
		}
	}

	VecBuffer* vbf = NULL;
	if (!strcmp("simd", alg)) {
		vbf = new SimdVecBuffer();
	} else if (!strcmp("simple", alg)) {
		vbf = new SimpleVecBuffer();
	} else if (!strcmp("simd64", alg)) {
		vbf = new Simd64VecBuffer();
	} else if (!strcmp("simple64", alg)) {
		vbf = new Simple64VecBuffer();
	} else if (!strcmp("none", alg)) {
		vbf = new VecBuffer();
	} else if (!strcmp("mtable", alg)) {
		vbf = new MTableVecBuffer();
	} else {
		vbf = NULL;
	}
	if (vbf != NULL) {
//		uint* allocate = gendata(portion, size);
		run(size, portion, vbf);
		delete vbf;
	}
}
