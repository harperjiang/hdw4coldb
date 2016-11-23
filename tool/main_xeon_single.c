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
#include <assert.h>
#include "../src/perf.h"
#include "../src/log.h"
#include "../src/timer.h"

// Join and print num matched
uint32_t match_counter;

void scan_dummy(uint32_t key, uint8_t* outer, uint8_t* inner, void*params) {
	match_counter++;
}

void process(uint32_t key, uint8_t* outer, uint8_t* inner) {
	match_counter++;
}

//
void xs_hash_access(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	match_counter = 0;
	log_info("Running hash...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	log_info("Running, load size %u...\n", loadsize);

	timer_token token;
	timer_start(&token);

	for (uint32_t i = 0; i < loadsize; i++) {
		entry* innerRecord = hash_get(table, keys[i]);
		if (innerRecord != NULL)
			process(keys[i], (uint8_t*) (keys + i), innerRecord->payload);
	}
	timer_stop(&token);

	log_info("hash running time: %u, matched row %u\n", token.wallclockms,
			match_counter);

	free(keys);
}

void xs_hash_scan(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	match_counter = 0;
	log_info("Running hash scan...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	log_info("Running, load size %u...\n", loadsize);

	scan_context context;
	context.func = scan_dummy;

	timer_token token;
	timer_start(&token);

	for (uint32_t i = 0; i < loadsize; i++) {
		hash_scan(table, keys[i], &context);
	}

	timer_stop(&token);

	log_info("hash scan running time: %u, matched row %u\n", token.wallclockms,
			match_counter);

	free(keys);
}

void xs_cht_access(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	match_counter = 0;
	log_info("Running cht...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

	// Run
	log_info("Running, load size %u...\n", loadsize);

	timer_token token;
	timer_start(&token);

	for (uint32_t i = 0; i < loadsize; i++) {
		cht_entry* entry = cht_find_uniq(table, keys[i]);
		if (NULL != entry) {
			process(keys[i], (uint8_t*) (keys + i), entry->payload);
		}
	}
	timer_stop(&token);

	log_info("cht running time: %u, matched row %u\n", token.wallclockms,
			match_counter);

	free(keys);
}

void xs_cht_scan(const char* buildfile, const char* loadfile) {
	srand(time(NULL));
	match_counter = 0;
	log_info("Running cht scan...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

// Run
	log_info("Running, load size %u...\n", loadsize);

	scan_context context;
	context.func = scan_dummy;

	timer_token token;
	timer_start(&token);

	for (uint32_t i = 0; i < loadsize; i++) {
		cht_scan(table, keys[i], &context);
	}

	timer_stop(&token);

	log_info("cht scan running time: %u, matched row %u\n", token.wallclockms,
			match_counter);

	free(keys);
}

void print_help() {
	fprintf(stdout,
			"Usage: main_xeon_single [-h] [-u] -b <key_file> -l <workload>\n");
	fprintf(stdout, " -h \tUse hash\n");
	fprintf(stdout, " -u \tUnique key\n");
	fprintf(stdout, " -b file Key file for building table\n");
	fprintf(stdout, " -l file Workload file\n");
}

int main(int argc, char** argv) {
	int c;
	bool uniq = false;
	bool hash = false;
	char* buildFile = NULL;
	char* loadFile = NULL;

	if (argc == 1) {
		print_help();
		exit(0);
	}

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
		xs_cht_scan(buildFile, loadFile);
		break;
	case 1:
		xs_cht_access(buildFile, loadFile);
		break;
	case 2:
		xs_hash_scan(buildFile, loadFile);
		break;
	case 3:
		xs_hash_access(buildFile, loadFile);
		break;
	default:
		abort();
	}
}
