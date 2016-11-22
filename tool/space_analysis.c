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

	cht* ct = (cht*) malloc(sizeof(cht));
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));

	perf_buildcht(ct, argv[1]);
	perf_buildhash(ht, argv[1]);

	printf("CHT: bitmap size %u\n", ct->bitmap_size);
	printf("CHT: payload size %u\n", ct->payload_size);
	printf("CHT: overflow bkt size %u\n", ct->overflow->bucket_size);
	printf("CHT: overflow size %u\n", ct->overflow->size);
	printf("Hash: bucket size %u\n", ht->bucket_size);
}

