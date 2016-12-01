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

	for (uint sIndex = 0; sIndex < splitRound; sIndex++) {
		CLBuffer* innerKeyBuffer = new CLBuffer(env,
				innerkey + sIndex * workSize, sizeof(uint32_t) * workSize,
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

void scan_chthash(uint* meta, ulong* bitmap, uint* chtPayload,
		uint* hashpayload, uint* inner, uint* result, uint index) {
	uint key = inner[index];
	uint bitmapSize = meta[0] * 32;
	uint payloadSize = meta[2];
	uint hash = (key * ((uint) 2654435761)) % bitmapSize;

	uint bitmapIndex = hash / BITMAP_UNIT;
	uint bitmapOffset = hash % BITMAP_UNIT;

	ulong bitmapMask = ~(0xffffffffffffffff << bitmapOffset);
	uint offset = (uint) (bitmap[bitmapIndex] >> 32)
			+ popcount((uint) (bitmap[bitmapIndex] & bitmapMask));

	uint i = 0;
	while (i < THRESHOLD && offset + i < payloadSize
			&& chtPayload[offset + i] != key) {
		i++;
	}
	if (offset + i < payloadSize && chtPayload[offset + i] == key) {
		//result[index] = offset + i;
		result[index] = 1;
		return;
	} else {
		// Search in Hash
		uint bucket_size = meta[1];
		if (0 == bucket_size) {
			result[index] = 3;
			return;
		}
		uint counter = (key * ((uint) 2654435761)) % bucket_size;
		while (hashpayload[counter] != key && hashpayload[counter] != 0) {
			counter = (counter + 1) % bucket_size;
		}
		if (hashpayload[counter] == key) {
			result[index] = 2;
		} else {
			result[index] = 3;
		}
	}

}

void runChtStep(kvlist* outer, kvlist* inner, uint split) {
	Timer timer;
	logger.info("Running CHT Step Join\n");
	logger.info("Building Outer Table\n");

	CHT* cht = new CHT();
	cht->build(outer->entries, outer->size);
	logger.info("Building Outer Table Done\n");

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

	CLEnv* env = new CLEnv();

	CLProgram* scanBitmap = new CLProgram(env, "scan_bitmap");
	scanBitmap->fromFile("scan_bitmap.cl", 4);
	CLProgram* scanCht = new CLProgram(env, "scan_chthash");
	scanCht->fromFile("scan_chthash.cl", 6);

	uint splitRound = 1;
	uint workSize = inner->size;
	if (split != 0) {
		splitRound = inner->size / split;
		splitRound += inner->size % split ? 1 : 0;
		workSize = split;
	}
	uint matched = 0;

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
			sizeof(uint32_t) * (meta[1] == 0 ? 1 : meta[1]),
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	uint32_t* passedkey = new uint32_t[workSize];

	for (uint sIndex = 0; sIndex < splitRound; sIndex++) {
		CLBuffer* innerkeyBuffer = new CLBuffer(env,
				innerkey + workSize * sIndex, sizeof(uint32_t) * workSize,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

		CLBuffer* bitmapResultBuffer = new CLBuffer(env, NULL,
				sizeof(char) * workSize, CL_MEM_READ_WRITE);

		scanBitmap->setBuffer(0, metaBuffer);
		scanBitmap->setBuffer(1, bitmapBuffer);
		scanBitmap->setBuffer(2, innerkeyBuffer);
		scanBitmap->setBuffer(3, bitmapResultBuffer);

		scanBitmap->execute(workSize);

		char* bitmapResult = (char*) bitmapResultBuffer->map(CL_MAP_READ);

		uint32_t counter = 0;
		for (uint32_t i = 0; i < workSize; i++) {
			if (bitmapResult[i]) {
				passedkey[counter++] = innerkey[sIndex * workSize + i];
			}
		}
		uint numPassBitmap = counter;
		bitmapResultBuffer->unmap();

		delete innerkeyBuffer;
		delete bitmapResultBuffer;

		CLBuffer* passbitmapKeyBuffer = new CLBuffer(env, passedkey,
				numPassBitmap * sizeof(uint32_t),
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				numPassBitmap * sizeof(uint32_t), CL_MEM_WRITE_ONLY);

		scanCht->setBuffer(0, metaBuffer);
		scanCht->setBuffer(1, bitmapBuffer);
		scanCht->setBuffer(2, chtpayloadBuffer);
		scanCht->setBuffer(3, hashpayloadBuffer);
		scanCht->setBuffer(4, passbitmapKeyBuffer);
		scanCht->setBuffer(5, resultBuffer);

		scanCht->execute(numPassBitmap);

		uint32_t* chtResult = (uint32_t*) resultBuffer->map(CL_MAP_READ);

		for (uint32_t i = 0; i < numPassBitmap; i++) {
			if (chtResult[i] != 0xffffffff) {
				matched++;
			}
		}
		resultBuffer->unmap();

		delete passbitmapKeyBuffer;
		delete resultBuffer;
	}
	timer.stop();

	logger.info("Running time: %u, matched row %u\n", timer.wallclockms(),
			matched);

	delete[] passedkey;
	delete[] hash_payload;
	delete[] cht_payload;
	delete[] innerkey;

	delete metaBuffer;
	delete bitmapBuffer;
	delete chtpayloadBuffer;
	delete hashpayloadBuffer;

	delete scanBitmap;
	delete scanCht;
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

	CLEnv* env = new CLEnv();

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
			sizeof(uint32_t) * (0 == cht->overflow->bucket_size) ?
					1 : cht->overflow->bucket_size,
			CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	uint32_t matched = 0;
	for (uint i = 0; i < splitRound; i++) {
		CLBuffer* innerkeyBuffer = new CLBuffer(env, innerkey + i * workSize,
				sizeof(uint32_t) * workSize,
				CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

		CLBuffer* resultBuffer = new CLBuffer(env, NULL,
				sizeof(uint32_t) * workSize, CL_MEM_READ_WRITE);

		scanChtFull->setBuffer(0, metaBuffer);
		scanChtFull->setBuffer(1, bitmapBuffer);
		scanChtFull->setBuffer(2, chtpayloadBuffer);
		scanChtFull->setBuffer(3, hashpayloadBuffer);
		scanChtFull->setBuffer(4, innerkeyBuffer);
		scanChtFull->setBuffer(5, resultBuffer);

		scanChtFull->execute(inner->size);

		uint32_t* result = (uint32_t*) resultBuffer->map(CL_MAP_READ);

		for (uint32_t i = 0; i < workSize; i++) {
			if (result[i] != 0xffffffff)
				matched++;
		}

		resultBuffer->unmap();

		delete innerkeyBuffer;
		delete resultBuffer;

	}
	timer.stop();

	logger.info("Running time: %u, matched row %u\n", timer.wallclockms(),
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

void print_help() {
	fprintf(stdout, "Usage: main_opencl [options]\n");
	fprintf(stdout, "Available options:\n");
	fprintf(stdout, " -a --alg=NAME	\tchoose algorithm\n");
	fprintf(stdout, " -o --outer=FILE \tfile for outer table\n");
	fprintf(stdout, " -i --inner=File \tfile for inner table\n");
	fprintf(stdout, " -h --help \tdisplay this information\n");
	fprintf(stdout, " -v --devinfo \tdisplay the detected device info\n");
	fprintf(stdout, " -s --split \tsplit the probe table\n");
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
					"split",
					optional_argument }, 0, 's' };

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
