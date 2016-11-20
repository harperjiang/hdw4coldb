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
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(ht->size, 0);
	ASSERT_EQ(ht->bucket_size, 32);

	for (int i = 0; i < 32; i++) {
		ASSERT_EQ(0, ht->buckets[i].key);
	}
}

TEST( Hash, Get) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}

	for (int i = 0; i < 32; i++) {
		entry* entry = hash_get(ht, i + 1);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}

}

TEST( Hash, Put) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}
}

uint32_t scan_counter;

void scan(uint32_t key, uint8_t* payload) {
	scan_counter++;
}

TEST( Hash, Scan) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 40);

	for (int i = 0; i < 40; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;
		// Only put 4 unique key, each key should have 10 items
		hash_put(ht, i % 4 + 1, value);
	}

	for (int i = 0; i < 40; i++) {
		scan_counter = 0;
		hash_scan(ht, i % 4 + 1, scan);
		ASSERT_EQ(10, scan_counter);
	}

}

TEST( Hash, Organize) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 100);

	for (int i = 0; i < 100; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}

	ASSERT_EQ(100, ht->size);
	ASSERT_TRUE(ht->bucket_size >= 100 * 1.5);

	for (int i = 0; i < 100; i++) {
		entry* entry = hash_get(ht, i + 1);

		ASSERT_EQ(i, entry->payload[0]);
		ASSERT_EQ(3, entry->payload[2]);
	}
}

