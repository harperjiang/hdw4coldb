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
#include "../vecbuffer/VecBuffer.h"
#include "../vecbuffer/SimdVecBuffer.h"
#include "../vecbuffer/SimpleVecBuffer.h"

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

void run(uint* data, uint size, VecBuffer* buffer) {
	int round = size / 8;
	int outputSize;

	Logger* logger = Logger::getLogger("perf_buffer");
	Timer timer;

	timer.start();

	for (int i = 0; i < round; i++) {
		__m256i input = _mm256_load_si256((__m256i *) (data + i * 8));
		buffer->serve(input, &outputSize);
	}
	buffer->purge(&outputSize);

	timer.stop();

	logger->info("Running size %u, time consumption %lu\n", size,
			timer.wallclockms());
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
					required_argument, 0, 'p' },
			{ "help", no_argument, 0, 'h' },
			{ "size", required_argument, 0, 's' } };

	int c;
	while ((c = getopt_long(argc, argv, "ap:hs:", long_options, &option_index))
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
	} else {
		vbf = NULL;
	}
	if (vbf != NULL) {
		uint* allocate = gendata(portion, size);
		run(allocate, size, vbf);
		free(allocate);
		delete vbf;
	}
}
