/*
 * UnitTest for Hash
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */
#include <gtest/gtest.h>
#include <stdlib.h>
#include "../src/Hash.h"

TEST( Hash, Build) {
	kv* entries = new kv[32]();

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	Hash* ht = new Hash();
	ht->build(entries, 32);

	delete [] entries;

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(32, ht->size());
	ASSERT_EQ(32 * RATIO, ht->bucketSize());
	delete ht;
}

TEST( Hash, Empty) {
	Hash* ht = new Hash(10);
	ASSERT_EQ(0, ht->size());
	ASSERT_EQ(10, ht->bucketSize());
	ASSERT_EQ(NULL, ht->get(65));

}

TEST( Hash, Get) {
	kv* entries = new kv[32]();

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	Hash* ht = new Hash();
	ht->build(entries, 32);

	delete [] entries;

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(ht->size(), 32);
	ASSERT_EQ(ht->bucketSize(), 32 * RATIO);

	for (int i = 0; i < 32; i++) {
		kv* entry = ht->get(i + 1);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}

	delete ht;
}

TEST( Hash, Put) {
	kv* entries = new kv[32]();

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	Hash* ht = new Hash();
	ht->build(entries, 32);

	delete [] entries;

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		ht->put(i + 128, value);
	}

	ASSERT_EQ(64, ht->size());
	delete ht;
}

uint32_t scan_counter;

void scan(uint32_t key, uint8_t* outer, uint8_t* inner, void* params) {
	scan_counter++;
}

TEST( Hash, Scan) {
	kv* entries = new kv[40]();

	for (int i = 0; i < 40; i++) {
		entries[i].key = i % 4 + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	Hash* ht = new Hash();
	ht->build(entries, 40);

	delete [] entries;

	ScanContext context(scan,NULL);

	for (int i = 0; i < 40; i++) {
		scan_counter = 0;
		ht->scan(i % 4 + 1, &context);
		ASSERT_EQ(10, scan_counter);
	}
	delete ht;
}

TEST( Hash, Organize) {
	kv* entries = new kv[32]();

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	Hash* ht = new Hash();
	ht->build(entries, 32);

	delete [] entries;

	for (int i = 0; i < 100; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		ht->put(i + 100, value);
	}

	ASSERT_EQ(132, ht->size());
	ASSERT_TRUE(ht->bucketSize() >= 132 * RATIO);

	for (int i = 0; i < 100; i++) {
		kv* entry = ht->get(i + 100);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}

	delete ht;
}

