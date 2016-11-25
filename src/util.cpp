/*
 * util.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */

#include "util.h"

void loadkey(const char* filename, kvlist* result) {
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

	kv *buffer = new kv[size];

	while ((read = getline(&line, &len, f)) != -1) {
		if (strlen(line) == 0)
			continue;
		uint32_t data = (uint32_t) strtoul(line, NULL, 10);
		buffer[counter].key = data;

		counter++;

		if (counter == size) {
			uint32_t newsize = size * 2;
			kv* new_buffer = new kv[newsize];
			::memcpy(new_buffer, buffer, size * sizeof(kv));
			kv* oldbuffer = buffer;
			buffer = new_buffer;
			delete []oldbuffer;
			size = newsize;
		}
	}

	fclose(f);
	result->entries = buffer;
	result->size = counter;
}
