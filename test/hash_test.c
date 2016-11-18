/*
 * hash_test.c
 *
 *  Created on: Nov 17, 2016
 *      Author: harper
 */
#include <gtest/gtest.h>
#include "../src/hash.h"

TEST(Hash, Build) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	ASSERT_TRUE(ht != NULL);
	ASSERT_EQ(ht->size, 0);
	ASSERT_EQ(ht->bucket_size, 32);

	for (int i = 0; i < 32; i++) {
		ASSERT_EQ(0, ht->buckets[i].key);
	}
}

TEST(Hash, Get) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}

	for (int i = 0; i < 32; i++) {
		uint8_t* data = hash_get(ht, i + 1);

		ASSERT_EQ(i, data[0]);
		ASSERT_EQ(3, data[2]);
	}

}

TEST(Hash, Put) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}
}

