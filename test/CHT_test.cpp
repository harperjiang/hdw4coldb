/*
 * UnitTest for CHT
 *
 *  Created on: Nov 18, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "../src/lookup/CHT.h"

#define BITMAP_FACTOR 8

using namespace std;

TEST( CHT, Build) {

	srand(time(NULL));

	CHT* table = new CHT();

	kv* entries = new kv[125000];

	for (int i = 0; i < 125000; i++) {
		entries[i].key = (uint32_t) rand();
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	table->build(entries, 125000);
	delete[] entries;

	ASSERT_TRUE(125000 >= table->payloadSize());
	ASSERT_EQ(125000 * BITMAP_FACTOR / 32, table->bitmapSize());

	delete table;
}

TEST( CHT, FindUnique) {
	srand(time(NULL));

	CHT* table = new CHT();

	kv* entries = new kv[125000];

	for (int i = 0; i < 125000; i++) {
		entries[i].key = i+1;
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	table->build(entries, 125000);

	for (int i = 0; i < 125000; i++) {
		kv* entry = table->findUnique(entries[i].key);
		ASSERT_TRUE(entry != NULL);
		for (int j = 0; j < 4; j++) {
			ASSERT_EQ(entries[i].payload[j], entry->payload[j]);
		}
	}
	delete[] entries;
	delete table;
}

uint32_t counter;
void myscanfunc(uint32_t key, uint8_t* outer, uint8_t* inner, void*param) {
	counter++;
}

TEST( CHT, Scan) {

	CHT* table = new CHT();

	kv* entries = new kv[50];

	for (int i = 0; i < 50; i++) {
		entries[i].key = (i%10)+1;
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	table->build(entries, 50);

	ScanContext context(myscanfunc,NULL);

	for (int i = 0; i < 50; i++) {
		context.updateInner(entries[i].payload);
		counter = 0;
		table->scan(entries[i].key, &context);
		ASSERT_EQ(5, counter);
	}

	delete[] entries;
	delete table;
}

TEST( CHT, Has) {
	CHT* table = new CHT();

	kv* entries = new kv[125000];

	for (int i = 0; i < 125000; i++) {
		entries[i].key = (uint32_t) rand();
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	table->build(entries, 125000);

	for (int i = 0; i < 125000; i++) {
		ASSERT_TRUE(table->has(entries[i].key));
	}

	delete[] entries;
	delete table;
}
