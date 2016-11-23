/*
 * UnitTest for CHT
 *
 *  Created on: Nov 18, 2016
 *      Author: harper
 */

#include <gtest/gtest.h>
#include <time.h>
#include <stdlib.h>
#include "../src/cht.h"

TEST( CHT, Build) {

	srand(time(NULL));

	cht* table = (cht*) malloc(sizeof(cht));

	kv* entries = (kv*) malloc(sizeof(kv) * 125000);

	for (int i = 0; i < 125000; i++) {
		entries[i].key = (uint32_t) rand();
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t)(rand() % 0xff);
		}
	}

	// Fill in random data
	cht_build(table, entries, 125000);
	free(entries);

	ASSERT_TRUE(125000 >= table->payload_size);
	ASSERT_EQ(125000 * 8 / 32, table->bitmap_size);

	cht_free(table);
}

TEST( CHT, FindUnique) {
	srand(time(NULL));

	cht* table = (cht*) malloc(sizeof(cht));

	kv* entries = (kv*) malloc(sizeof(kv) * 125000);

	for (int i = 0; i < 125000; i++) {
		entries[i].key = i + 1;
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t)(rand() % 0xff);
		}
	}

	// Fill in random data
	cht_build(table, entries, 125000);
	free(entries);

	for (int i = 0; i < 125000; i++) {
		cht_entry* entry = cht_find_uniq(table, entries[i].key);
		for (int j = 0; j < 4; j++) {
			ASSERT_EQ(entries[i].payload[j], entry->payload[j]);
		}
	}

	cht_free(table);
}

TEST( CHT, FindAll) {

}

TEST( CHT, Has) {
	srand(time(NULL));

	cht* table = (cht*) malloc(sizeof(cht));
	kv* entries = (kv*) malloc(sizeof(kv) * 125000);
	uint32_t key_counter = 1;
	for (int i = 0; i < 125000; i++) {
		entries[i].key = key_counter++;
	}

	cht_build(table, entries, 125000);
	free(entries);
	for (int i = 0; i < 125000; i++) {
		ASSERT_TRUE(cht_has(table, entries[i].key));
	}

	cht_free(table);
}
