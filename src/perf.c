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

void perf_loadkey(const char* filename, kvlist* result) {
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

	kv *buffer = (kv*) malloc(sizeof(kv) * size);

	while ((read = getline(&line, &len, f)) != -1) {
		if (strlen(line) == 0)
			continue;
		uint32_t data = (uint32_t) strtoul(line, NULL, 10);
		buffer[counter].key = data;

		counter++;

		if (counter == size) {
			uint32_t newsize = size * 2;
			kv* new_buffer = (kv*) malloc(sizeof(kv) * newsize);
			memcpy(new_buffer, buffer, size * sizeof(kv));
			kv* oldbuffer = buffer;
			buffer = new_buffer;
			free(oldbuffer);
			size = newsize;
		}
	}

	fclose(f);
	result->entries = buffer;
	result->size = size;
}
