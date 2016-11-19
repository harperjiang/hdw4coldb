/**
 * Performance Comparison
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include "../src/perf.h"

void cht_scan_dummy(cht_entry *entry) {

}

void hash_scan_dummy(entry* entry) {

}
//
void hash_access(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	printf("Running hash...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Warm up
	printf("Warming up...\n");
	perf_hash_access(table, loadsize, keys);
	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	perf_hash_access(table, loadsize, keys);
	clock_t end = clock();

	double running_time = end - start / (CLOCKS_PER_SEC);

	printf("hash running time: %f\n", running_time);
}

void hash_scan(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	printf("Running hash scan...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Warm up
	printf("Warming up...\n");
	perf_hash_access(table, loadsize, keys);
	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	perf_hash_scan(table, loadsize, keys, hash_scan_dummy);
	clock_t end = clock();

	double running_time = end - start / (CLOCKS_PER_SEC);

	printf("hash scan running time: %f\n", running_time);
}

void cht_access(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	printf("Running cht...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Warm up
	printf("Warming up...\n");
	perf_cht_access(table, loadsize, keys);
	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	perf_cht_access(table, loadsize, keys);
	clock_t end = clock();

	double running_time = end - start / (CLOCKS_PER_SEC);

	printf("cht running time: %f\n", running_time);
}

void cht_scan(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	printf("Running cht scan...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Warm up
	printf("Warming up...\n");
	perf_cht_access(table, loadsize, keys);
	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	perf_cht_scan(table, loadsize, keys, cht_scan_dummy);
	clock_t end = clock();

	double running_time = end - start / (CLOCKS_PER_SEC);

	printf("cht scan running time: %f\n", running_time);
}

int main(int argc, char** argv) {
	int c;
	bool uniq = false;
	bool hash = false;
	char* buildFile;
	char* loadFile;

	while ((c = getopt(argc, argv, "hub:l:")) != -1)
		switch (c) {
		case 'u':
			uniq = true;
			break;
		case 'h':
			hash = true;
			break;
		case 'b':
			buildFile = optarg;
			break;
		case 'l':
			loadFile = optarg;
			break;
		case '?':
			if (optopt == 's' || optopt == 'b' || optopt == 'l')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	switch ((hash & 1) << 1 | (uniq & 1)) {
	case 0:
		cht_scan(buildFile, loadFile);
		break;
	case 1:
		cht_access(buildFile, loadFile);
		break;
	case 2:
		hash_scan(buildFile, loadFile);
		break;
	case 3:
		hash_access(buildFile, loadFile);
		break;
	default:
		abort();
	}
}
