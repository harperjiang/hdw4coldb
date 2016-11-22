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

	cht_entry* entries = (cht_entry*) malloc(sizeof(cht_entry) * size);
	for (uint32_t i = 0; i < size; i++) {
		entries[i].key = keys[i];
		memcpy(entries[i].payload, (uint8_t*) (keys + i),
				sizeof(uint8_t) * PAYLOAD_SIZE);
	}

	cht_build(table, entries, size);
}

void perf_buildhash(hashtable* table, const char* filename) {
	uint32_t size;
	uint32_t* keys = perf_loadkey(filename, &size);

	hash_build(table, size * RATIO + 1);

	for (uint32_t i = 0; i < size; i++) {
		hash_put(table, keys[i], (uint8_t*) (keys + i));
	}
}

