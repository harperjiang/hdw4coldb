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
#include <semaphore.h>

#include "lookup/Lookup.h"
#include "lookup/Hash.h"
#include "lookup/CHT.h"
#include "lookup/LookupHelper.h"
#include "util/Logger.h"
#include "util/Timer.h"

typedef struct _thread_arg {
	Lookup* lookup;
	kvlist* inner;
	uint32_t start;
	uint32_t stop;
	bool uniq;
	uint32_t result;
	sem_t* sema;
} thread_arg;

Logger* logger = Logger::getLogger("mthread");

// Join and print num matched
void scan_func(uint32_t key, uint8_t* outer, uint8_t* inner, void* params) {
	((thread_arg*) params)->result++;
}

void process(uint32_t key, uint8_t* outer, uint8_t* inner, uint32_t* result) {
	(*result)++;
}

/**
 * Partition the key to <pnum> strides
 */
void partition(kvlist* key, uint32_t pnum, thread_arg* partitions) {
	// Direct split to pnum pieces
	for (uint32_t i = 0; i < pnum; i++) {
		partitions[i].inner = key;
		partitions[i].start = (key->size / pnum) * i;
		partitions[i].stop = (key->size / pnum) * (i + 1);
	}
	partitions[pnum - 1].stop = key->size - 1;
}

void run_thread(pthread_t* threads, thread_arg* args, uint32_t numthread,
		Lookup* table, kvlist* inner, bool uniq, void* (*thread_func)(void*)) {
	sem_t semaphore;
	sem_init(&semaphore, 0, 0);

	partition(inner, numthread, args);

	for (uint32_t i = 0; i < numthread; i++) {
		args[i].lookup = table;
		args[i].uniq = uniq;
		args[i].sema = &semaphore;
		args[i].result = 0;
		pthread_create(threads + i, NULL, thread_func, (void*) (args + i));
	}

	// Wait for end
	for (uint32_t i = 0; i < numthread; i++) {
		sem_wait(&semaphore);
	}
	sem_destroy(&semaphore);
}

void* xm_thread_access(void* arg) {
	thread_arg *context = (thread_arg*) arg;
	context->result = 0;

	Lookup* alg = context->lookup;

	if (context->uniq) {
		for (uint32_t i = context->start; i < context->stop; i++) {
			kv inner = context->inner->entries[i];
			uint8_t* outer = alg->access(inner.key);
			if (NULL != outer) {
				process(inner.key, outer, inner.payload, &context->result);
			}
		}
	} else {
		ScanContext sc(scan_func, context);
		for (uint32_t i = context->start; i < context->stop; i++) {
			sc.updateInner(context->inner->entries[i].payload);
			alg->scan(context->inner->entries[i].key, &sc);
		}
	}
	sem_post(context->sema);
	pthread_exit(NULL);
	return NULL;
}

void xm_access(Lookup* lookup, kvlist* outer, kvlist* inner, uint32_t numthread,
		bool uniq) {
	srand(time(NULL));

	logger->info("Running %s join with %d threads\n", lookup->getName(),
			numthread);

	logger->info("Building outer table\n");
	lookup->build(outer->entries, outer->size);
	logger->info("Building outer table done\n");

	// Run

	Timer timer;
	timer.start();

	pthread_t threads[numthread];
	thread_arg args[numthread];

	run_thread(threads, args, numthread, lookup, inner, uniq, xm_thread_access);

	// Collect result
	uint32_t match_counter = 0;
	for (uint32_t i = 0; i < numthread; i++) {
		match_counter += args[i].result;
	}

	timer.stop();

	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			match_counter);
}

void print_help() {
	fprintf(stdout, "Usage: mtjoin [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -t --thread=NUM\tnumber of threads\n");
	fprintf(stdout, " -u --unique	\touter is unique\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	exit(0);
}

int main(int argc, char** argv) {
	bool uniq = false;
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;
	uint32_t numthread = 1;

	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "unique", no_argument, 0,
					'u' }, { "outer", required_argument, 0, 'o' }, { "inner",
			required_argument, 0, 'i' }, { "help", no_argument, 0, 'h' }, {
					"thread", required_argument, 0, 't' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:uo:i:ht:", long_options,
			&option_index)) != -1) {
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
		case 't':
			numthread = strtoul(optarg, NULL, 0);
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

	xm_access(lookup, &outerkeys, &innerkeys, numthread, uniq);

	delete[] outerkeys.entries;
	delete[] innerkeys.entries;

	delete lookup;
}
