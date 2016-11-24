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
#include <unistd.h>
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
void xs_access(algo* algo_obj, kvlist* outerfile, kvlist* innerfile,
bool uniq) {
	srand(time(NULL));

	log_info("Running %s join\n", algo_obj->prototype->name);
	log_info("Building outer table\n");
	algo_obj->prototype->build(algo_obj, outerfile->entries, outerfile->size);
	log_info("Building outer table done\n");
	timer_token token;
	timer_start(&token);

	if (uniq) {
		for (uint32_t i = 0; i < innerfile->size; i++) {
			kv inner = innerfile->entries[i];
			uint8_t* outerpl = algo_obj->prototype->access(algo_obj, inner.key);
			if (outerpl != NULL)
				process(inner.key, outerpl, inner.payload);
		}
	} else {
		scan_context sc;
		sc.func = scan_dummy;
		for (uint32_t i = 0; i < innerfile->size; i++) {
			sc.inner = innerfile->entries[i].payload;
			algo_obj->prototype->scan(algo_obj, innerfile->entries[i].key, &sc);
		}
	}
	timer_stop(&token);

	log_info("Running time: %u, matched row %u\n", token.wallclockms,
			match_counter);
}

void print_help() {
	fprintf(stdout, "Usage: main_xeon_single [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -u --unique	\touter is unique\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -h --help \t\tdisplay this information\n");
	exit(0);
}

int main(int argc, char** argv) {

	bool uniq = false;
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;

	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "unique", no_argument, 0,
					'u' }, { "outer", required_argument, 0, 'o' }, { "inner",
			required_argument, 0, 'i' }, { "help", no_argument, 0, 'h' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:uo:i:h", long_options, &option_index))
			!= -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'u':
			uniq = true;
			break;
		case 'o':
			outerfile = optarg;
			break;
		case 'i':
			innerfile = optarg;
			break;
		case 'h':
			print_help();
			break;
		default:
			fprintf(stderr, "unrecognized option or missing argument\n");
			print_help();
			break;
		}
	}

	init_class();

	kvlist outerkeys;
	kvlist innerkeys;
	log_info("Loading files\n");
	perf_loadkey(outerfile, &outerkeys);
	perf_loadkey(innerfile, &innerkeys);
	log_info("Outer file size: %u\n", outerkeys.size);
	log_info("Inner file size: %u\n", innerkeys.size);

	algo* algo;
	if (!strcmp("hash", alg)) {
		algo = hash_algo_new();
	} else if (!strcmp("cht", alg)) {
		algo = cht_algo_new();
	} else {
		algo = cht_algo_new();
	}

	xs_access(algo, &outerkeys, &innerkeys, uniq);

	free(outerkeys.entries);
	free(innerkeys.entries);
}
