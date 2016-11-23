/*
 * main_xeon_multi.c
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "../src/perf.h"

uint8_t* outer;

// Join and print
void scan_dummy(uint32_t key, uint8_t* payload) {
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		assert(((uint8_t* ) &key)[i] == payload[i]);
	}
	fprintf(stdout, "%u\n", key);
}

void process(uint32_t key, uint8_t* outer, uint8_t* inner) {
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		assert(inner[i] == outer[i]);
	}
	fprintf(stdout, "%u\n", key);
}

//
void hash_access(const char* buildfile, const char* loadfile,
		uint32_t numthread) {
	srand(time(NULL));
	printf("Running hash...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	for (uint32_t i = 0; i < loadsize; i++) {
		entry* outerRecord = hash_get(table, keys[i]);
		if (outerRecord != NULL)
			process(keys[i], outerRecord->payload, (uint8_t*) (keys + i));
	}
	clock_t end = clock();

	double running_time = (end - start) / (CLOCKS_PER_SEC);

	printf("hash running time: %f\n", running_time);
}

void hash_scan(const char* buildfile, const char* loadfile, uint32_t numthread) {
	srand(time(NULL));
	printf("Running hash scan...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	for (uint32_t i = 0; i < loadsize; i++) {
		hash_scan(table, keys[i], scan_dummy);
	}
	clock_t end = clock();

	double running_time = (end - start) / (CLOCKS_PER_SEC);

	printf("hash scan running time: %f\n", running_time);
}

void cht_access(const char* buildfile, const char* loadfile, uint32_t numthread) {
	srand(time(NULL));
	printf("Running cht...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	for (uint32_t i = 0; i < loadsize; i++) {
		cht_entry* entry = cht_find_uniq(table, keys[i]);
		if (NULL != entry) {
			process(keys[i], entry->payload, (uint8_t*) (keys + i));
		}
	}
	clock_t end = clock();

	double running_time = (end - start) / (CLOCKS_PER_SEC);

	printf("cht running time: %f\n", running_time);
}

void cht_scan(const char* buildfile, const char* loadfile, uint32_t numthread) {
	srand(time(NULL));
	printf("Running cht scan...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	printf("Running, load size %u...\n", loadsize);
	clock_t start = clock();
	for (uint32_t i = 0; i < loadsize; i++) {
		cht_scan(table, keys[i], scan_dummy);
	}
	clock_t end = clock();

	double running_time = (end - start) / (CLOCKS_PER_SEC);

	printf("cht scan running time: %f\n", running_time);
}

void print_help() {
	fprintf(stdout,
			"Usage: main_xeon_multi [-h] [-u] -t <num_thread> -b <key_file> -l <workload>\n");
	fprintf(stdout, " -h \tUse hash\n");
	fprintf(stdout, " -u \tUnique key\n");
	fprintf(stdout, " -t num  Number of threads\n");
	fprintf(stdout, " -b file Key file for building table\n");
	fprintf(stdout, " -l file Workload file\n");
}

int main(int argc, char** argv) {
	int c;
	bool uniq = false;
	bool hash = false;
	char* buildFile = NULL;
	char* loadFile = NULL;
	uint32_t numthread = 1;

	if (argc == 1) {
		print_help();
		exit(0);
	}

	while ((c = getopt(argc, argv, "hub:l:t:")) != -1)
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
		case 't':
			numthread = strtoul(optarg, NULL, 10);
			break;
		case '?':
			if (optopt == 's' || optopt == 'b' || optopt == 'l' || optopt = 't')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			print_help();
			return 1;
		default:
			print_help();
			exit(1);
		}
	if (NULL == buildFile || NULL == loadFile) {
		print_help();
		exit(1);
	}
	switch ((hash & 1) << 1 | (uniq & 1)) {
	case 0:
		cht_scan(buildFile, loadFile, numthread);
		break;
	case 1:
		cht_access(buildFile, loadFile, numthread);
		break;
	case 2:
		hash_scan(buildFile, loadFile, numthread);
		break;
	case 3:
		hash_access(buildFile, loadFile, numthread);
		break;
	default:
		abort();
	}
}
