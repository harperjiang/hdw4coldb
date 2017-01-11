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
#include "../filter/Filter.h"
#include "../filter/ScalarFilter.h"
#include "../filter/SimdFilter.h"

void run(uint size, Filter* filter) {
	srand(time(NULL));

	uint* input = (uint*) aligned_alloc(32, size * sizeof(uint));
	uint* output = (uint*) aligned_alloc(32, size * sizeof(uint));

	for (uint i = 0; i < size; i++) {
		input[i] = rand() % 10;
	}

	Logger* logger = Logger::getLogger("perf_buffer");
	Timer timer;

	timer.start();

	filter->filter(input, size, output);

	timer.stop();

	logger->info("Running size %u, time consumption %lu\n", size,
			timer.wallclockms());

}

void print_help() {
	fprintf(stdout, "Usage: buffer_perf [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg alg_name \tAlgorithm to use\n");
	fprintf(stdout, " -s --size \tsize of test\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	exit(0);
}

int main(int argc, char** argv) {
	Logger* logger = Logger::getLogger("pred_perf");

	uint size = 1000000;
	char* alg;

	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' },
			{ "help", no_argument, 0, 'h' },
			{ "size", required_argument, 0, 's' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:hs:", long_options, &option_index))
			!= -1) {
		switch (c) {
		case 'a':
			alg = optarg;
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

	number num;
	num.integer = 5;
	Filter* filter = NULL;
	if (!strcmp("simd", alg)) {
		filter = new SimdFilter(EQ, num);
	} else if (!strcmp("scalar", alg)) {
		filter = new ScalarFilter(EQ, num);
	} else {
		filter = NULL;
	}
	if (filter != NULL) {
		run(size, filter);
		delete filter;
	}
}
