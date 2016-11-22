/*
 * mergedata.c
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

void print_help() {
	fprintf(stderr, "Usage: mergedata total_size f1 ratio1 f2 ratio2\n");
}

int main(int argc, char** argv) {
	if (argc != 6) {
		print_help();
		exit(1);
	}
	uint32_t size = (uint32_t) strtoul(argv[1], NULL, 10);
	char* key1file = argv[2];
	uint32_t key1ratio = (uint32_t) strtoul(argv[3], NULL, 10);
	char* key2file = argv[4];
	uint32_t key2ratio = (uint32_t) strtoul(argv[5], NULL, 10);

	FILE* key1f = fopen(key1file, "r");
	FILE* key2f = fopen(key2file, "r");
	if (key1f == NULL || key2f == NULL)
		exit(EXIT_FAILURE);

	srand(time(NULL));

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	uint32_t totalRatio = key1ratio + key2ratio;

	uint32_t counter = 0;
	bool leftDrained = false;
	bool rightDrained = false;

	while (counter < size) {
		if (leftDrained && rightDrained) {
			fprintf(stderr, "no line in either file\n");
			abort();
		}
		uint32_t r = rand() % totalRatio;
		if (rightDrained || (!leftDrained && r <= key1ratio)) {
			read = getline(&line, &len, key1f);
			if (read == -1) {
				leftDrained = true;
				continue;
			} else {
				fprintf(stdout, "%s", line);
				counter++;
			}
		} else if (leftDrained || (!rightDrained && r > key1ratio)) {
			read = getline(&line, &len, key2f);
			if (read == -1) {
				rightDrained = true;
				continue;
			} else {
				fprintf(stdout, "%s", line);
				counter++;
			}
		}
	}

	fclose(key1f);
	fclose(key2f);
}
