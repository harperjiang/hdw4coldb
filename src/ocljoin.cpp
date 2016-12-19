#include "ocljoin.h"

using namespace std;

extern void runHash(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling = false);

extern void runCht(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling = false);

void print_help() {
	fprintf(stdout, "Usage: ocljoin [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -p --profiling \tenable profiling\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	fprintf(stdout, " -v --devinfo \tdisplay the detected device info\n");
	fprintf(stdout, " -s --split \tsplit the probe table\n");
	exit(0);
}

void display_device() {
	CLEnv* env = new CLEnv();
	env->displayDeviceInfo();

	delete env;
	exit(0);
}

int main(int argc, char** argv) {
	Logger* logger = Logger::getLogger("ocljoin");
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;
	bool enableProfiling = false;
	uint split = 0;
	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "outer", required_argument,
					0, 'o' }, { "inner", required_argument, 0, 'i' }, { "help",
			no_argument, 0, 'h' }, { "devinfo", no_argument, 0, 'v' }, {
					"split",
					optional_argument, 0, 's' }, { "profiling",
			no_argument, 0, 'p' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:o:i:hvs:p", long_options,
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
		case 's':
			split = strtoul(optarg, NULL, 10);
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

	kvlist outerkeys;
	kvlist innerkeys;
	if (outerfile != NULL && innerfile != NULL) {
		logger->info("Loading files\n");
		loadkey(outerfile, &outerkeys);
		loadkey(innerfile, &innerkeys);
		logger->info("Outer file size: %u\n", outerkeys.size);
		logger->info("Inner file size: %u\n", innerkeys.size);
	}
	if (enableProfiling) {
		Logger::getLogger("CLBuffer")->setLevel(DEBUG);
		Logger::getLogger("CLProgram")->setLevel(DEBUG);
	}

	if (!strcmp("hash", alg)) {
		runHash(&outerkeys, &innerkeys, split, enableProfiling);
	} else if (!strcmp("cht", alg)) {
		runCht(&outerkeys, &innerkeys, split, enableProfiling);
	}

	delete[] outerkeys.entries;
	delete[] innerkeys.entries;

}
