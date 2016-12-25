using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>

#include "lookup/LookupHelper.h"
#include "join/Join.h"
#include "join/simple/HashJoin.h"
#include "join/simple/CHTJoin.h"
#include "join/ocl/OclHashJoin.h"
#include "join/ocl/OclCHTJoin.h"
#include "join/simd/SimdCHTJoin.h"
#include "join/cstep/OclStepCHTJoin.h"

void print_help() {
	fprintf(stdout, "Usage: join [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -p --profiling \tenable profiling\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	fprintf(stdout, " -v --devinfo \tdisplay the detected device info\n");
	fprintf(stdout, " -t --numthread \tnumber of threads to use\n");
	exit(0);
}

void display_device() {
	CLEnv* env = new CLEnv();
	env->displayDeviceInfo();

	delete env;
	exit(0);
}

int main(int argc, char** argv) {
	Logger* logger = Logger::getLogger("join");
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;
	bool enableProfiling = false;
	uint numThread = 0;
	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "outer", required_argument,
					0, 'o' }, { "inner", required_argument, 0, 'i' }, { "help",
			no_argument, 0, 'h' }, { "devinfo", no_argument, 0, 'v' }, {
					"numthread", required_argument, 0, 't' }, { "profiling",
			no_argument, 0, 'p' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:o:i:hvs:pt:", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'p':
			enableProfiling = true;
			break;
		case 'o':
			outerfile = optarg;
			break;
		case 'i':
			innerfile = optarg;
			break;
		case 't':
			numThread = strtoul(optarg, NULL, 10);
			break;
		case 'h':
			print_help();
			break;
		case 'v':
			display_device();
			break;
		default:
			fprintf(stderr, "unrecognized option or missing argument\n");
			print_help();
			break;
		}
	}

	if (enableProfiling) {
		Logger::getLogger("CLBuffer")->setLevel(DEBUG);
		Logger::getLogger("CLProgram")->setLevel(DEBUG);
	}
	Join* join = NULL;
	if (!strcmp("hash", alg)) {
		join = new HashJoin(numThread, enableProfiling);
	} else if (!strcmp("cht", alg)) {
		join = new CHTJoin(numThread, enableProfiling);
	} else if (!strcmp("oclhash", alg)) {
		join = new OclHashJoin(enableProfiling);
	} else if (!strcmp("oclcht", alg)) {
		join = new OclCHTJoin(enableProfiling);
	} else if (!strcmp("simdcht", alg)) {
		join = new SimdCHTJoin(true, true, enableProfiling);
	} else if (!strcmp("oclstepcht", alg)) {
		join = new OclStepCHTJoin(true, true, enableProfiling);
	} else {
		join = NULL;
	}
	if (NULL != join) {
		kvlist outerkeys;
		kvlist innerkeys;
		if (outerfile != NULL && innerfile != NULL) {
			logger->info("Loading files\n");
			loadkey(outerfile, &outerkeys);
			loadkey(innerfile, &innerkeys);
			logger->info("Outer file size: %u\n", outerkeys.size);
			logger->info("Inner file size: %u\n", innerkeys.size);
		}

		join->getLogger()->setLevel(DEBUG);
		join->join(&outerkeys, &innerkeys);
	} else {
		logger->error("Algorithm %s not recognized\n", alg);
	}
	delete[] outerkeys.entries;
	delete[] innerkeys.entries;

}
