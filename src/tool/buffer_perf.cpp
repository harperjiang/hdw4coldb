/*
 * buffer_perf.cpp
 *
 *  Created on: Jan 4, 2017
 *      Author: harper
 */

#include <sys/types.h>
#include "../util/Logger.h"

void print_help() {
	fprintf(stdout, "Usage: buffer_perf [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -b --usebuffer	\tuse SimdBuffer\n");
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

	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "usebuffer", no_argument, 0, 'b' }, { "portion",
					required_argument, 0, 'p' },
			{ "help", no_argument, 0, 'h' },
			{ "size", required_argument, 0, 's' } };

	int c;
	while ((c = getopt_long(argc, argv, "bp:hs:", long_options, &option_index))
			!= -1) {
		switch (c) {
		case 'b':
			useBuffer = true;
			break;
		case 'p':
			break;
		case 'h':
			print_help();
			break;
		case 's':
			break;
		default:
			fprintf(stderr, "unrecognized option or missing argument\n");
			print_help();
			break;
		}
	}

}
