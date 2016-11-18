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

TEST( Hash, PutDuplicate) {
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
		entry* ent = hash_get(ht, i % 4 + 1);
		while (ent != NULL && ent->payload[0] != i) {
			ent = ent->next;
		}
		ASSERT_TRUE(ent != NULL);
		ASSERT_EQ(i, ent->payload[0]);
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

