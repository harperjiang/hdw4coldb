/*
 * space_analysis.c
 *
 *  Created on: Nov 22, 2016
 *      Author: harper
 */

#include <stdio.h>
#include <stdlib.h>
#include "../src/perf.h"
#include "../src/cht.h"
#include "../src/hash.h"

int main(int argc, char** argv) {

	cht* cht = (cht*) malloc(sizeof(cht));
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));

	perf_buildcht(cht, argv[1]);
	perf_buildhash(ht, argv[1]);

	printf("CHT: bitmap size %u\n", cht->bitmap_size);
	printf("CHT: payload size %u\n", cht->payload_size);
	printf("CHT: overflow bkt size %u\n", cht->overflow->bucket_size);
	printf("CHT: overflow size %u\n", cht->overflow->size);
	printf("Hash: bucket size %u\n", ht->bucket_size);
}

