/**********************************************************************
 Copyright ?015 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 ?Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 ?Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************/

// For clarity,error checking has been omitted.
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>

#include "../src/Timer.h"
#include "../src/CHT.h"
#include "../src/Logger.h"
#include "../src/opencl/CLEnv.h"
#include "../src/opencl/CLProgram.h"
#include "../src/util.h"

#define THRESHOLD 5
#define BITMAP_FACTOR 	4
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

using namespace std;

Logger logger;

extern bool bitmap_test(uint64_t* bitmap, uint32_t offset);
extern uint32_t bitmap_popcnt(uint64_t* bitmap, uint32_t offset);

void runHash(kvlist* outer, kvlist* inner, uint split) {
	logger.info("Running hash join\n");
	Hash* hash = new Hash();
	hash->build(outer->entries, outer->size);

	CLEnv* env = new CLEnv();

	CLProgram* hashScan = new CLProgram(env, "scan_hash");
	hashScan->fromFile("scan_hash.cl", 4);

	uint32_t meta[2];
	meta[1] = hash->bucket_size;
	uint32_t* payload = new uint32_t[hash->bucket_size];
	for (uint32_t i = 0; i < hash->bucket_size; i++) {
		payload[i] = hash->buckets[i].key;
	}

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	Timer timer;
	timer.start();

	uint splitRound = 1;
	uint workSize = inner->size;
	if (split != 0) {
		splitRound = inner->size / split;
		workSize = split;
		while (workSize * splitRound < inner->size)
			splitRound++;
	}

	uint matched = 0;

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* payloadBuffer = new CLBuffer(env, payload,
			sizeof(uint32_t) * hash->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	for (uint i = 0; i < splitRound; i++) {
		CLBuffer* innerKeyBuffer = new CLBuffer(env, innerkey + i * workSize,
				sizeof(uint32_t) * workSize,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * workSize, CL_MEM_WRITE_ONLY);

		hashScan->setBuffer(0, metaBuffer);
		hashScan->setBuffer(1, payloadBuffer);
		hashScan->setBuffer(2, innerKeyBuffer);
		hashScan->setBuffer(3, resultBuffer);

		hashScan->execute(workSize);

		uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);
		for (uint32_t i = 0; i < workSize; i++) {
			matched += result[i] == 0xffffffff ? 0 : 1;
		}
		resultBuffer->unmap();

		delete innerKeyBuffer;
		delete resultBuffer;
	}

	timer.stop();
	logger.info("Running time: %u, matched row %u\n", timer.wallclockms(),
			matched);

	delete metaBuffer;
	delete payloadBuffer;

	delete hashScan;
	delete env;
	delete hash;
}

void runChtStep(kvlist* outer, kvlist* inner, uint split) {
	Timer timer;
	logger.info("Running CHT Step Join\n");
	logger.info("Building Outer Table\n");
	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger.info("Building Outer Table Done\n");
	uint32_t meta[2];

	meta[0] = cht->bitmap_size;

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	uint32_t* cht_payload = new uint32_t[cht->payload_size];
	for (uint32_t i = 0; i < cht->payload_size; i++) {
		cht_payload[i] = cht->payloads[i].key;
	}

	uint32_t* hash_payload = new uint32_t[cht->overflow->bucket_size];
	for (uint32_t i = 0; i < cht->overflow->bucket_size; i++) {
		hash_payload[i] = cht->overflow->buckets[i].key;
	}

	timer.start();

	CLEnv* env = new CLEnv();

	CLProgram* scanBitmap = new CLProgram(env, "scan_bitmap");
	scanBitmap->fromFile("scan_bitmap.cl", 4);
	CLProgram* scanCht = new CLProgram(env, "scan_cht");
	scanCht->fromFile("scan_cht.cl", 5);
	CLProgram* scanHash = new CLProgram(env, "scan_hash");
	scanHash->fromFile("scan_hash.cl", 4);

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
	CLBuffer* bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey,
			sizeof(uint32_t) * inner->size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint32_t) * inner->size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint32_t) * cht->overflow->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* bitmapResultBuffer = new CLBuffer(env, NULL,
			sizeof(char) * inner->size, CL_MEM_READ_WRITE);

	scanBitmap->setBuffer(0, metaBuffer);
	scanBitmap->setBuffer(1, bitmapBuffer);
	scanBitmap->setBuffer(2, innerkeyBuffer);
	scanBitmap->setBuffer(3, bitmapResultBuffer);

	scanBitmap->execute(inner->size);

	char* bitmapResult = (char*) bitmapResultBuffer->map(CL_MAP_READ);

	uint32_t* passBitmap = new uint32_t[inner->size];

	uint32_t counter = 0;
	for (uint32_t i = 0; i < inner->size; i++) {
		if (bitmapResult[i] && cht->has(innerkey[i])) {
			passBitmap[counter++] = innerkey[i];
		}
	}
	uint numPassBitmap = counter;
	bitmapResultBuffer->unmap();

	CLBuffer* passbitmapKeyBuffer = new CLBuffer(env, passBitmap,
			numPassBitmap * sizeof(uint32_t),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* chtResultBuffer = new CLBuffer(env, NULL,
			numPassBitmap * sizeof(uint32_t), CL_MEM_WRITE_ONLY);

	scanCht->setBuffer(0, metaBuffer);
	scanCht->setBuffer(1, bitmapBuffer);
	scanCht->setBuffer(2, chtpayloadBuffer);
	scanCht->setBuffer(3, passbitmapKeyBuffer);
	scanCht->setBuffer(4, chtResultBuffer);

	scanCht->execute(numPassBitmap);

	uint32_t* chtResult = (uint32_t*) chtResultBuffer->map(CL_MAP_READ);

	uint32_t* failedCht = new uint32_t[numPassBitmap];

	counter = 0;
	uint32_t matched = 0;
	for (uint32_t i = 0; i < numPassBitmap; i++) {
		if (0xffffffff == chtResult[i]) {
			failedCht[counter++] = passBitmap[i];
		} else {
			matched++;
		}
	}
	chtResultBuffer->unmap();

	uint32_t numFailedCht = counter;

	CLBuffer* failedchtkeyBuffer = new CLBuffer(env, failedCht,
			numFailedCht * sizeof(uint32_t),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* finalResultBuffer = new CLBuffer(env, NULL,
			numFailedCht * sizeof(uint32_t), CL_MEM_WRITE_ONLY);

	scanHash->setBuffer(0, metaBuffer);
	scanHash->setBuffer(1, hashpayloadBuffer);
	scanHash->setBuffer(2, failedchtkeyBuffer);
	scanHash->setBuffer(3, finalResultBuffer);

	scanHash->execute(numFailedCht);

	uint32_t* result = (uint32_t*) finalResultBuffer->map(CL_MAP_READ);

	for (uint32_t i = 0; i < numFailedCht; i++) {
		if (result[i] != 0xffffffff)
			matched++;
	}

	finalResultBuffer->unmap();

	timer.stop();

	logger.info("Running time: %u, matched row %u\n", timer.wallclockms(),
			matched);

	delete[] failedCht;
	delete[] passBitmap;
	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete innerkeyBuffer;
	delete bitmapResultBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;
	delete chtResultBuffer;
	delete failedchtkeyBuffer;
	delete finalResultBuffer;

	delete scanBitmap;
	delete scanCht;
	delete scanHash;
	delete env;

	delete cht;
}

void runCht(kvlist* outer, kvlist* inner, uint split) {
	Timer timer;
	logger.info("Running CHT Join\n");
	logger.info("Building Outer Table\n");
	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger.info("Building Outer Table Done\n");

	uint32_t meta[2];
	meta[0] = cht->bitmap_size;
	meta[1] = cht->overflow->bucket_size;

	uint32_t* innerkey = new uint32_t[inner->size];
	for (uint32_t i = 0; i < inner->size; i++) {
		innerkey[i] = inner->entries[i].key;
	}

	uint32_t* cht_payload = new uint32_t[cht->payload_size];
	for (uint32_t i = 0; i < cht->payload_size; i++) {
		cht_payload[i] = cht->payloads[i].key;
	}

	uint32_t* hash_payload = new uint32_t[cht->overflow->bucket_size];
	for (uint32_t i = 0; i < cht->overflow->bucket_size; i++) {
		hash_payload[i] = cht->overflow->buckets[i].key;
	}

	timer.start();

	CLEnv* env = new CLEnv();

	CLProgram* scanChtFull = new CLProgram(env, "scan_cht_full");
	scanChtFull->fromFile("scan_cht_full.cl", 6);

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 2,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
	CLBuffer* bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint32_t) * inner->size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint32_t) * cht->overflow->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey,
			sizeof(uint32_t) * inner->size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* resultBuffer = new CLBuffer(env, NULL,
			sizeof(uint32_t) * inner->size, CL_MEM_READ_WRITE);

	scanChtFull->setBuffer(0, metaBuffer);
	scanChtFull->setBuffer(1, bitmapBuffer);
	scanChtFull->setBuffer(2, chtpayloadBuffer);
	scanChtFull->setBuffer(3, hashpayloadBuffer);
	scanChtFull->setBuffer(4, innerkeyBuffer);
	scanChtFull->setBuffer(5, resultBuffer);

	scanChtFull->execute(inner->size);

	uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

	uint32_t matched = 0;
	for (uint32_t i = 0; i < inner->size; i++) {
		if (result[i] != 0xffffffff)
			matched++;
	}

	resultBuffer->unmap();

	timer.stop();

	logger.info("Running time: %u, matched row %u\n", timer.wallclockms(),
			matched);

	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete innerkeyBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;
	delete resultBuffer;

	delete scanChtFull;
	delete env;

	delete cht;
}

void print_help() {
	fprintf(stdout, "Usage: main_opencl [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	fprintf(stdout, " -v --devinfo \tdisplay the detected device info\n");
	exit(0);
}

void display_device() {
	CLEnv* env = new CLEnv();
	env->logger.setLevel(DEBUG);
	env->displayDeviceInfo();

	delete env;
	exit(0);
}

int main(int argc, char** argv) {
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;
	uint split = 0;
	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "outer", required_argument,
					0, 'o' }, { "inner",
			required_argument, 0, 'i' }, { "help",
			no_argument, 0, 'h' }, { "devinfo", no_argument, 0, 'v' }, {
					"split", required_argument }, 0, 's' };

	int c;
	while ((c = getopt_long(argc, argv, "a:o:i:hvs:", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'o':
			outerfile = optarg;
			break;
		case 'i':
			innerfile = optarg;
			break;
		case 's':
			split = strtoul(optarg, NULL, 10);
			break;
		case 'h':
			print_help();
			break;
		case 'v':
			display_device();
			break;
		default:
			fprintf(stderr, "unrecognized option or missing argument\n");
			print_help();
			break;
		}
	}

	Logger logger;

	kvlist outerkeys;
	kvlist innerkeys;
	logger.info("Loading files\n");
	loadkey(outerfile, &outerkeys);
	loadkey(innerfile, &innerkeys);
	logger.info("Outer file size: %u\n", outerkeys.size);
	logger.info("Inner file size: %u\n", innerkeys.size);

	if (!strcmp("hash", alg)) {
		runHash(&outerkeys, &innerkeys, split);
	} else if (!strcmp("chtstep", alg)) {
		runChtStep(&outerkeys, &innerkeys, split);
	} else if (!strcmp("cht", alg)) {
		runCht(&outerkeys, &innerkeys, split);
	}

	delete[] outerkeys.entries;
	delete[] innerkeys.entries;

}
