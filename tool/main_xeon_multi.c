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
#include "../src/perf.h"
#include "../src/log.h"
#include "../src/timer.h"

typedef struct _thread_arg {
	void* table;
	uint32_t* key;
	uint32_t start;
	uint32_t stop;
	uint32_t result;
	sem_t* sema;
} thread_arg;

// Join and print num matched

void scan_func(uint32_t key, uint8_t* outer, uint8_t* inner, void* params) {
	((thread_arg*) params)->result++;
}

void process(uint32_t key, uint8_t* outer, uint8_t* inner, uint32_t* result) {
	(*result)++;
}

void partition(uint32_t* key, uint32_t keysize, uint32_t pnum,
		thread_arg* partitions) {
	// Direct split to pnum pieces
	for (uint32_t i = 0; i < pnum; i++) {
		partitions[i].key = key;
		partitions[i].start = (keysize / pnum) * i;
		partitions[i].stop = (keysize / pnum) * (i + 1);
	}
	partitions[pnum - 1].stop = keysize - 1;
}

void run_thread(pthread_t* threads, thread_arg* args, uint32_t numthread,
		void* table, uint32_t* keys, uint32_t keysize,
		void* (*thread_func)(void*)) {
	sem_t semaphore;
	sem_init(&semaphore, 0, 0);

	partition(keys, keysize, numthread, args);

	for (uint32_t i = 0; i < numthread; i++) {
		args[i].table = table;
		args[i].sema = &semaphore;
		pthread_create(threads + i, NULL, thread_func, (void*) (args + i));
	}

// Wait for end
	for (uint32_t i = 0; i < numthread; i++) {
		sem_wait(&semaphore);
	}
	sem_destroy(&semaphore);
}

void* xm_hash_thread_access(void* arg) {
	thread_arg *context = (thread_arg*) arg;
	context->result = 0;
	for (uint32_t i = context->start; i < context->stop; i++) {
		entry* entry = hash_get((hashtable*) context->table, context->key[i]);
		if (NULL != entry) {
			process(context->key[i], NULL/*should be outer payload here*/,
					entry->payload, &context->result);
		}
	}
	sem_post(context->sema);
	pthread_exit(NULL);
}

void* xm_hash_thread_scan(void *arg) {
	thread_arg *context = (thread_arg*) arg;
	context->result = 0;

	scan_context sc;
	sc.func = scan_func;
	sc.params = context;
	for (uint32_t i = context->start; i < context->stop; i++) {
		sc.inner = NULL;
		hash_scan((hashtable*) context->table, context->key[i], &sc);
	}
	sem_post(context->sema);
	pthread_exit(NULL);
}

void xm_hash(const char* buildfile, const char* loadfile, uint32_t numthread,
bool scan) {
	srand(time(NULL));
	log_info("Running hash...\n");
	hashtable* table = (hashtable*) malloc(sizeof(hashtable));

	perf_buildhash(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

// Run
	log_info("Running, load size %u...\n", loadsize);

	timer_token token;
	timer_start(&token);

	pthread_t threads[numthread];
	thread_arg args[numthread];

	if (scan)
		run_thread(threads, args, numthread, table, keys, loadsize,
				xm_hash_thread_scan);
	else
		run_thread(threads, args, numthread, table, keys, loadsize,
				xm_hash_thread_access);

// Collect result
	uint32_t match_counter = 0;
	for (uint32_t i = 0; i < numthread; i++) {
		match_counter += args[i].result;
	}

	timer_stop(&token);

	log_info("hash running time: %u, matched row %u\n", token.wallclockms,
			match_counter);

	free(keys);
}

void* xm_cht_thread_access(void* arg) {
	thread_arg *context = (thread_arg*) arg;
	context->result = 0;
	for (uint32_t i = context->start; i < context->stop; i++) {
		cht_entry* entry = cht_find_uniq((cht*) context->table,
				context->key[i]);
		if (NULL != entry) {
			process(context->key[i], NULL/*should be outer payload here*/,
					entry->payload, &context->result);
		}
	}
	sem_post(context->sema);
	pthread_exit(NULL);
}

void* xm_cht_thread_scan(void *arg) {
	thread_arg *context = (thread_arg*) arg;
	context->result = 0;

	scan_context sc;
	sc.func = scan_func;
	sc.params = context;
	for (uint32_t i = context->start; i < context->stop; i++) {
		sc.inner = NULL;
		cht_scan((cht*) context->table, context->key[i], &sc);
	}
	sem_post(context->sema);
	pthread_exit(NULL);
}

void xm_cht(const char* buildfile, const char* loadfile, uint32_t numthread,
bool scan) {
	srand(time(NULL));
	log_info("Running cht...\n");
	cht* table = (cht*) malloc(sizeof(cht));

	perf_buildcht(table, buildfile);

	uint32_t loadsize;
	uint32_t* keys = perf_loadkey(loadfile, &loadsize);

// Run
	log_info("Running, load size %u...\n", loadsize);

	timer_token token;
	timer_start(&token);

	pthread_t threads[numthread];
	thread_arg args[numthread];

	if (scan)
		run_thread(threads, args, numthread, table, keys, loadsize,
				xm_cht_thread_scan);
	else
		run_thread(threads, args, numthread, table, keys, loadsize,
				xm_cht_thread_access);

// Collect result
	uint32_t match_counter = 0;
	for (uint32_t i = 0; i < numthread; i++) {
		match_counter += args[i].result;
	}

	timer_stop(&token);

	log_info("cht running time: %u, threads: %u, matched row %u\n",
			token.wallclockms, numthread, match_counter);

	free(keys);
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
			if (optopt == 's' || optopt == 'b' || optopt == 'l'
					|| optopt == 't')
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
		xm_cht(buildFile, loadFile, numthread, true);
		break;
	case 1:
		xm_cht(buildFile, loadFile, numthread, false);
		break;
	case 2:
		xm_hash(buildFile, loadFile, numthread, true);
		break;
	case 3:
		xm_hash(buildFile, loadFile, numthread, false);
		break;
	default:
		abort();
	}
}
