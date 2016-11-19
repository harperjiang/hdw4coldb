/*
 * exp_prep.c
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "perf.h"

uint32_t* load_key(const char* filename, uint32_t* sizeholder) {
	FILE* f = fopen(filename, "r");

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	if (f == NULL)
		exit(EXIT_FAILURE);

	uint32_t counter = 0;
	uint32_t size = 1000;

	uint32_t *buffer = (uint32_t*) malloc(sizeof(uint32_t) * size);

	while ((read = getline(&line, &len, f)) != -1) {
		uint32_t data = (uint32_t) strtoul(line, NULL, 10);
		buffer[counter++] = data;

		if (counter == size) {
			uint32_t newsize = size * 2;
			uint32_t* new_buffer = (uint32_t*) malloc(
					sizeof(uint32_t) * newsize);
			memcpy(new_buffer, buffer, size);
			buffer = new_buffer;
			size = newsize;
		}
	}

	fclose(f);
	*sizeholder = size;
	return buffer;
}

void load_cht(cht* table, const char* filename) {
	uint32_t size;
	uint32_t* keys = load_key(filename, &size);

	cht_entry* entries = (cht_entry*) malloc(sizeof(cht_entry) * size);
	for (uint32_t i = 0; i < size; i++) {
		entries[i].key = keys[i];
	}

	cht_build(table, entries, size);
}

void load_hash(hashtable* table, const char* filename) {
	uint32_t size;
	uint32_t* keys = load_key(filename, &size);

	hash_build(table, size * RATIO + 1);

	uint8_t load[4];
	for (uint32_t i = 0; i < size; i++) {
		hash_put(table, keys[i], load);
	}
}

void perf_warmupcht(cht* table, uint32_t size) {
	for (uint32_t i = 0; i < size; i++) {
		cht_find_uniq(table, ((uint32_t) rand()));
	}
}

void perf_warmuphash(hashtable* table, uint32_t size) {
	for (uint32_t i = 0; i < size; i++) {
		hash_get(table, ((uint32_t) rand()));
	}
}
