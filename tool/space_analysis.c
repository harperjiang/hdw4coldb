/*
 * space_analysis.c
 *
 *  Created on: Nov 22, 2016
 *      Author: harper
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "../src/algo.h"
#include "../src/perf.h"
#include "../src/cht.h"
#include "../src/hash.h"

int main(int argc, char** argv) {

	int c;
	char* alg = NULL;
	char* file = NULL;
	while ((c = getopt(argc, argv, "a:o:")) != -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'o':
			file = optarg;
			break;
		default:
			exit(1);
		}
	}

	init_class();

	kvlist outer;

	perf_loadkey(file, &outer);

	if (!strcmp("hash", alg)) {
		algo* algobj = hash_algo_new();
		algobj->prototype->build(algobj, outer.entries, outer.size);
		hashtable* ht = (hashtable*) algobj;
		printf("Hash: bucket size %u\n", ht->bucket_size);
	} else if (!strcmp("cht", alg)) {
		algo* algobj = cht_algo_new();
		algobj->prototype->build(algobj, outer.entries, outer.size);
		cht* ct = (cht*) algobj;
		printf("CHT: bitmap size %u\n", ct->bitmap_size);
		printf("CHT: payload size %u\n", ct->payload_size);
		printf("CHT: overflow bkt size %u\n", ct->overflow->bucket_size);
		printf("CHT: overflow size %u\n", ct->overflow->size);
	} else {
		exit(1);
	}

	free(outer.entries);
}

