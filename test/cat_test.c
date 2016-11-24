/*
 * UnitTest for CHT
 *
 *  Created on: Nov 18, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <time.h>
#include <stdlib.h>
#include "../src/cat.h"

TEST( CAT, Build) {

	srand(time(NULL));

	cat* table = (cat*) malloc(sizeof(cat));

	kv* entries = (kv*) malloc(sizeof(kv) * 125000);

	for (uint32_t i = 0; i < 125000; i++) {
		entries[i].key = i + 5000;
		for (uint32_t j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t)(rand() % 0xff);
		}
	}

	// Fill in random data
	cat_build(table, entries, 125000);

	ASSERT_TRUE(125000 == table->payload_size);
	ASSERT_EQ(5000, table->min);
	ASSERT_EQ(129999, table->max);
	ASSERT_EQ(1 + 125000 / 32, table->bitmap_size);

	cat_free(table);
}

TEST( CAT, FindUnique) {
	srand(time(NULL));

	cat* table = (cat*) malloc(sizeof(cat));

	kv* entries = (kv*) malloc(sizeof(kv) * 125000);

	for (uint32_t i = 0; i < 125000; i++) {
		entries[i].key = i + 5000;
		for (uint32_t j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t)(rand() % 0xff);
		}
	}

	// Fill in random data
	cat_build(table, entries, 125000);

	for (int i = 0; i < 125000; i++) {
		uint8_t* data = cat_find_uniq(table, entries[i].key);
		for (int j = 0; j < 4; j++) {
			ASSERT_EQ(data[j], entries[i].payload[j]);
		}
	}

	cat_free(table);
}

TEST( CAT, Has) {
	srand(time(NULL));

	cat* table = (cat*) malloc(sizeof(cat));

	kv* entries = (kv*) malloc(sizeof(kv) * 125000);

	for (uint32_t i = 0; i < 125000; i++) {
		entries[i].key = i + 5000;
		for (uint32_t j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t)(rand() % 0xff);
		}
	}

	// Fill in random data
	cat_build(table, entries, 125000);

	for (int i = 0; i < 125000; i++) {
		uint8_t* data = cat_find_uniq(table, entries[i].key);
		for (int j = 0; j < 4; j++) {
			ASSERT_EQ(data[j], entries[i].payload[j]);
		}
	}

	cat_free(table);
}
