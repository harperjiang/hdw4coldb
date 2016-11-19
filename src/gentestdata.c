/*
 * gentestdata.c
 *
 *  Created on: Nov 19, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

void gen_unique(uint32_t size, FILE* f) {
	srand(time(NULL));
	uint32_t max = (uint32_t) 0xffffffffffffffff;
	uint32_t fold = max / size;

	uint32_t counter = 0;
	uint32_t pointer = 0;
	while (counter < size) {
		uint32_t rm = (size - counter);
		uint32_t rn = (max - pointer);
		if (((uint32_t) rand()) % rn < rm) {
			// choose counter
			uint32_t infold = ((uint32_t) rand()) % fold;
			fprintf(f, "%u\n", infold * size + pointer);
			counter++;
		}
		pointer++;
	}
}

void gen_near_unique(uint32_t size, FILE* f) {
	uint32_t copy_range = 5;
	uint32_t unique_range = 10;
	uint32_t unique_rate = 8;

	srand(time(NULL));
	uint32_t max = (uint32_t) 0xffffffffffffffff;
	uint32_t fold = max / size;

	uint32_t counter = 0;
	uint32_t pointer = 0;
	while (counter < size) {
		uint32_t rm = (size - counter);
		uint32_t rn = (max - pointer);
		if (((uint32_t) rand()) % rn < rm) {
			// choose counter
			uint32_t infold = ((uint32_t) rand()) % fold;

			if ((rand() % unique_range) > unique_rate) {
				uint32_t copy = (((uint32_t) rand()) % copy_range) + 1;
				for (uint32_t i = 0; i < copy; i++)
					fprintf(f, "%u\n", infold * size + pointer);
				counter += copy;
			} else {
				fprintf(f, "%u\n", infold * size + pointer);
				counter += 1;
			}
		}
		pointer++;
	}
}
/*
int main(int argc, char** argv) {
	int c;
	bool uniq = false;
	int size;
	char* outFile;

	while ((c = getopt(argc, argv, "us:o:")) != -1)
		switch (c) {
		case 'u':
			uniq = true;
			break;
		case 's':
			size = atoi(optarg);
			break;
		case 'o':
			outFile = optarg;
			break;
		case '?':
			if (optopt == 's' || optopt == 'o')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}

	FILE* file = fopen(outFile, "w");
	if (uniq)
		gen_unique(size, file);
	else
		gen_near_unique(size, file);
	fclose(file);
}
*/
