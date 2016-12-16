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
#include "lookup/Lookup.h"
#include "lookup/Hash.h"
#include "lookup/CHT.h"
#include "lookup/LookupHelper.h"
#include "util/Logger.h"
#include "util/Timer.h"

// Join and print num matched
uint32_t match_counter;

Logger* logger = Logger::getLogger("singlethread");

void scan_dummy(uint32_t key, uint8_t* outer, uint8_t* inner, void*params) {
	match_counter++;
}

void process(uint32_t key, uint8_t* outer, uint8_t* inner) {
	match_counter++;
}

//
void xs_access(Lookup* lookup, kvlist* outerfile, kvlist* innerfile,
		bool uniq) {
	srand(time(NULL));

	logger->info("Running %s join\n", lookup->getName());
	logger->info("Building outer table\n");
	lookup->build(outerfile->entries, outerfile->size);
	logger->info("Building outer table done\n");
	Timer timer;
	timer.start();

	match_counter = 0;

	if (uniq) {
		for (uint32_t i = 0; i < innerfile->size; i++) {
			kv inner = innerfile->entries[i];
			uint8_t* outerpl = lookup->access(inner.key);
			if (outerpl != NULL)
				process(inner.key, outerpl, inner.payload);
		}
	} else {
		ScanContext sc(scan_dummy, NULL);
		for (uint32_t i = 0; i < innerfile->size; i++) {
			sc.updateInner(innerfile->entries[i].payload);
			lookup->scan(innerfile->entries[i].key, &sc);
		}
	}
	timer.stop();

	logger->info("Running time: %u, matched row %u\n", timer.wallclockms(),
			match_counter);
}

void print_help() {
	fprintf(stdout, "Usage: stjoin [options]\n");
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

	kvlist outerkeys;
	kvlist innerkeys;
	logger->info("Loading files\n");
	loadkey(outerfile, &outerkeys);
	loadkey(innerfile, &innerkeys);
	logger->info("Outer file size: %u\n", outerkeys.size);
	logger->info("Inner file size: %u\n", innerkeys.size);

	Lookup* lookup;
	if (!strcmp("hash", alg)) {
		lookup = new Hash();
	} else if (!strcmp("cht", alg)) {
		lookup = new CHT();
	} else {
		lookup = new CHT();
	}

	xs_access(lookup, &outerkeys, &innerkeys, uniq);

	delete[] outerkeys.entries;
	delete[] innerkeys.entries;
	delete lookup;
}
