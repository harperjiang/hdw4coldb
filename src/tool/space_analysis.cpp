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
#include "../lookup/Hash.h"
#include "../lookup/CHT.h"
#include "../lookup/LookupHelper.h"

#include "../util/Logger.h"

Logger* logger = Logger::getLogger("space_analysis");

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

	kvlist outer;

	loadkey(file, &outer);

	if (!strcmp("hash", alg)) {
		Hash *hash = new Hash();
		hash->build(outer.entries, outer.size);
		logger->info("Hash: bucket size %u\n", hash->bucketSize());
		delete hash;
	} else if (!strcmp("cht", alg)) {
		CHT* cht = new CHT();
		cht->build(outer.entries, outer.size);
		logger->info("CHT: bitmap size %u\n", cht->bitmapSize());
		logger->info("CHT: payload size %u\n", cht->payloadSize());
		logger->info("CHT: overflow bkt size %u\n",
				cht->getOverflow()->bucketSize());
		logger->info("CHT: overflow size %u\n", cht->getOverflow()->size());
		delete cht;
	} else {
		exit(1);
	}

	delete[] outer.entries;
}

