/*
 * exp_prep.c
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include "perf.h"

uint32_t* perf_loadkey(const char* filename, uint32_t* sizeholder) {
	FILE* f = fopen(filename, "r");

	if (NULL == f) {
		fprintf(stderr, "Cannot open file %s\n", filename);
		exit(1);
	}
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	uint32_t counter = 0;
	uint32_t size = 1000;

	uint32_t *buffer = (uint32_t*) malloc(sizeof(uint32_t) * size);

	while ((read = getline(&line, &len, f)) != -1) {
		if (strlen(line) == 0)
			continue;
		uint32_t data = (uint32_t) strtoul(line, NULL, 10);
		buffer[counter++] = data;

		if (counter == size) {
			uint32_t newsize = size * 2;
			uint32_t* new_buffer = (uint32_t*) malloc(
					sizeof(uint32_t) * newsize);
			memcpy(new_buffer, buffer, size * sizeof(uint32_t));
			uint32_t* oldbuffer = buffer;
			buffer = new_buffer;
			free(oldbuffer);
			size = newsize;
		}
	}

	fclose(f);
	*sizeholder = counter;
	return buffer;
}

void perf_buildcht(cht* table, const char* filename) {
	uint32_t size;
	uint32_t* keys = perf_loadkey(filename, &size);

	kv* entries = (kv*) malloc(sizeof(kv) * size);
	for (uint32_t i = 0; i < size; i++) {
		entries[i].key = keys[i];
		memcpy(entries[i].payload, (uint8_t*) (keys + i),
				sizeof(uint8_t) * PAYLOAD_SIZE);
	}

	cht_build(table, entries, size);
	free(entries);
	free(keys);
}

void perf_buildhash(hashtable* table, const char* filename) {
	uint32_t size;
	uint32_t* keys = perf_loadkey(filename, &size);
	kv* entries = (kv*) malloc(sizeof(kv) * size);
	for (uint32_t i = 0; i < size; i++) {
		entries[i].key = keys[i];
		memcpy(entries[i].payload, (uint8_t*) (keys + i),
				sizeof(uint8_t) * PAYLOAD_SIZE);
	}

	hash_build(table, entries, size);
	free(entries);
	free(keys);
}

pthread_key_t current_workload;
pthread_key_t scan_func;

void perf_scan_wrapper(uint32_t key, uint8_t* outer) {
	kv* workload = (kv*) pthread_getspecific(current_workload);
	void (*func)( uint32_t, uint8_t*, uint8_t*);
	func = (void (*)(uint32_t, uint8_t*, uint8_t*)) pthread_getspecific(
			scan_func);
	func(key, outer, workload->payload);
}

void perf_scancht(cht* table, kv* workload, uint32_t size,
		void (*scanfunc)( uint32_t, uint8_t*, uint8_t*)) {
	pthread_setspecific(scan_func, (void*) scanfunc);
	for (uint32_t i = 0; i < size; i++) {
		pthread_setspecific(current_workload, (void*) (workload + i));
		cht_scan(table, workload[i].key, perf_scan_wrapper);
	}
}

void perf_scanhash(hashtable* table, kv* workload, uint32_t size,
		void (*scanfunc)( uint32_t, uint8_t*, uint8_t*)) {

}
