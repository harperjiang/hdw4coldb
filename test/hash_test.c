/*
 * UnitTest for Hash
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */
#include <gtest/gtest.h>
#include "../src/hash.h"
#include <stdlib.h>

TEST( Hash, Build) {
	kv* entries = (kv*) malloc(sizeof(kv) * 32);

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, entries, 32);

	free(entries);

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(32, ht->size);
	ASSERT_EQ(32 * RATIO, ht->bucket_size);
	hash_free(ht);
}

TEST( Hash, Get) {
	kv* entries = (kv*) malloc(sizeof(kv) * 32);

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, entries, 32);

	free(entries);

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(ht->size, 32);
	ASSERT_EQ(ht->bucket_size, 32 * RATIO);

	for (int i = 0; i < 32; i++) {
		entry* entry = hash_get(ht, i + 1);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}

	hash_free(ht);
}

TEST( Hash, Put) {
	kv* entries = (kv*) malloc(sizeof(kv) * 32);

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, entries, 32);
	free(entries);
	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 128, value);
	}

	ASSERT_EQ(64, ht->size);
	hash_free(ht);
}

uint32_t scan_counter;

void scan(uint32_t key, uint8_t* payload) {
	scan_counter++;
}

TEST( Hash, Scan) {
	kv* entries = (kv*) malloc(sizeof(kv) * 40);

	for (int i = 0; i < 40; i++) {
		entries[i].key = i % 4 + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, entries, 40);
	free(entries);

	for (int i = 0; i < 40; i++) {
		scan_counter = 0;
		hash_scan(ht, i % 4 + 1, scan);
		ASSERT_EQ(10, scan_counter);
	}
	hash_free(ht);
}

TEST( Hash, Organize) {
	kv* entries = (kv*) malloc(sizeof(kv) * 32);

	for (int i = 0; i < 32; i++) {
		entries[i].key = i + 1;
		entries[i].payload[0] = i;
		entries[i].payload[2] = 3;
	}

	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, entries, 32);
	free(entries);

	for (int i = 0; i < 100; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 100, value);
	}

	ASSERT_EQ(132, ht->size);
	ASSERT_TRUE(ht->bucket_size >= 132 * RATIO);

	for (int i = 0; i < 100; i++) {
		entry* entry = hash_get(ht, i + 100);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}

	hash_free(ht);
}

