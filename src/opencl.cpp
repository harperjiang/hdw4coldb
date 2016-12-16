// For clarity,error checking has been omitted.
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>

#include "util/Timer.h"
#include "lookup/CHT.h"
#include "util/Logger.h"
#include "opencl/CLEnv.h"
#include "opencl/CLProgram.h"
#include "lookup/LookupHelper.h"

#define THRESHOLD 5
#define BITMAP_SIZE		32
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

using namespace std;

Logger* logger = Logger::getLogger("opencl");

extern void runChtStep(kvlist* outer, kvlist* inner, uint split,
		bool enableProfiling);

void runHash(kvlist* outer, kvlist* inner, uint split, bool enableProfiling =
		false) {
	logger->info("Running hash join\n");
	Hash* hash = new Hash();
	hash->build(outer->entries, outer->size);

	CLEnv* env = new CLEnv(enableProfiling);

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

	for (uint sIndex = 0; sIndex < splitRound; sIndex++) {
		uint offset = sIndex * workSize;
		uint length =
				(sIndex + 1) * workSize > inner->size ?
						inner->size - sIndex * workSize : workSize;

		CLBuffer* innerKeyBuffer = new CLBuffer(env, innerkey + offset,
				sizeof(uint32_t) * length,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * length, CL_MEM_WRITE_ONLY);

		hashScan->setBuffer(0, metaBuffer);
		hashScan->setBuffer(1, payloadBuffer);
		hashScan->setBuffer(2, innerKeyBuffer);
		hashScan->setBuffer(3, resultBuffer);

		hashScan->execute(length);

		uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);
		for (uint32_t i = 0; i < length; i++) {
			matched += result[i] == 0xffffffff ? 0 : 1;
		}
		resultBuffer->unmap();

		delete innerKeyBuffer;
		delete resultBuffer;
	}

	timer.stop();
	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			matched);

	delete metaBuffer;
	delete payloadBuffer;

	delete hashScan;
	delete env;
	delete hash;
}

void runCht(kvlist* outer, kvlist* inner, uint split, bool enableProfiling =
		false) {
	Timer timer;
	logger->info("Running CHT Join\n");
	logger->info("Building Outer Table\n");
	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger->info("Building Outer Table Done\n");

	uint32_t meta[3];
	meta[0] = cht->bitmap_size;
	meta[1] = cht->overflow->bucket_size;
	meta[2] = cht->payload_size;

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

	CLEnv* env = new CLEnv(enableProfiling);

	CLProgram* scanChtFull = new CLProgram(env, "scan_cht_full");
	scanChtFull->fromFile("scan_cht_full.cl", 6);

	uint splitRound = 1;
	uint workSize = inner->size;
	if (split != 0) {
		splitRound = inner->size / split;
		splitRound += inner->size % split ? 1 : 0;
		workSize = split;
	}

	timer.start();

	CLBuffer* metaBuffer = new CLBuffer(env, meta, sizeof(uint32_t) * 3,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);

	CLBuffer* bitmapBuffer = new CLBuffer(env, cht->bitmap,
			sizeof(uint64_t) * cht->bitmap_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* chtpayloadBuffer = new CLBuffer(env, cht_payload,
			sizeof(uint32_t) * cht->payload_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	CLBuffer* hashpayloadBuffer = new CLBuffer(env, hash_payload,
			sizeof(uint32_t)
					* ((0 == cht->overflow->bucket_size) ?
							1 : cht->overflow->bucket_size),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	uint32_t matched = 0;
	for (uint sIndex = 0; sIndex < splitRound; sIndex++) {
		uint offset = sIndex * workSize;
		uint length =
				(sIndex + 1) * workSize > inner->size ?
						inner->size - sIndex * workSize : workSize;

		CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey + offset,
				sizeof(uint32_t) * length,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * length,
				CL_MEM_READ_WRITE);
		CLBuffer* debugBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * length,
				CL_MEM_READ_WRITE);
		scanChtFull->setBuffer(0, metaBuffer);
		scanChtFull->setBuffer(1, bitmapBuffer);
		scanChtFull->setBuffer(2, chtpayloadBuffer);
		scanChtFull->setBuffer(3, hashpayloadBuffer);
		scanChtFull->setBuffer(4, innerkeyBuffer);
		scanChtFull->setBuffer(5, resultBuffer);

		scanChtFull->execute(length);

		uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

		for (uint32_t i = 0; i < length; i++) {
			if (result[i] != 0xffffffff)
				matched++;
		}

		resultBuffer->unmap();

		delete innerkeyBuffer;
		delete resultBuffer;
		delete debugBuffer;
	}
	timer.stop();

	logger->info("Running time: %u ms, matched row %u\n", timer.wallclockms(),
			matched);

	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;

	delete scanChtFull;
	delete env;

	delete cht;
}

void runExperiment() {
	CLEnv* env = new CLEnv();
	CLProgram* program = new CLProgram(env, "test_bitmap");

	program->fromFile("test_bitmap.cl", 1);

	CLBuffer* resultBuffer = new CLBuffer(env, NULL, sizeof(uint),
	CL_MEM_READ_WRITE);
	program->setBuffer(0, resultBuffer);

	program->execute(32);

	uint* result = (uint*) resultBuffer->map(CL_MAP_READ);

	logger->info("%x\n", result[0]);

	delete program;
	delete env;
}

void print_help() {
	fprintf(stdout, "Usage: opencl [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -p --profiling \tenable profiling\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	fprintf(stdout, " -v --devinfo \tdisplay the detected device info\n");
	fprintf(stdout, " -s --split \tsplit the probe table\n");
	exit(0);
}

void display_device() {
	CLEnv* env = new CLEnv();
	env->displayDeviceInfo();

	delete env;
	exit(0);
}

int main(int argc, char** argv) {
	char* alg = NULL;
	char* outerfile = NULL;
	char* innerfile = NULL;
	bool enableProfiling = false;
	uint split = 0;
	if (argc == 1) {
		print_help();
		exit(0);
	}

	int option_index = 0;
	static struct option long_options[] = {
			{ "alg", required_argument, 0, 'a' }, { "outer", required_argument,
					0, 'o' }, { "inner", required_argument, 0, 'i' }, { "help",
			no_argument, 0, 'h' }, { "devinfo", no_argument, 0, 'v' }, {
					"split",
					optional_argument, 0, 's' }, { "profiling",
			no_argument, 0, 'p' } };

	int c;
	while ((c = getopt_long(argc, argv, "a:o:i:hvs:p", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 'a':
			alg = optarg;
			break;
		case 'p':
			enableProfiling = true;
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

	kvlist outerkeys;
	kvlist innerkeys;
	if (outerfile != NULL && innerfile != NULL) {
		logger->info("Loading files\n");
		loadkey(outerfile, &outerkeys);
		loadkey(innerfile, &innerkeys);
		logger->info("Outer file size: %u\n", outerkeys.size);
		logger->info("Inner file size: %u\n", innerkeys.size);
	}
	if (enableProfiling) {
		Logger::getLogger("CLBuffer")->setLevel(DEBUG);
		Logger::getLogger("CLProgram")->setLevel(DEBUG);
	}

	if (!strcmp("hash", alg)) {
		runHash(&outerkeys, &innerkeys, split, enableProfiling);
	} else if (!strcmp("chtstep", alg)) {
		runChtStep(&outerkeys, &innerkeys, split, enableProfiling);
	} else if (!strcmp("cht", alg)) {
		runCht(&outerkeys, &innerkeys, split, enableProfiling);
	} else {
		runExperiment();
	}
	delete[] outerkeys.entries;
	delete[] innerkeys.entries;

}
