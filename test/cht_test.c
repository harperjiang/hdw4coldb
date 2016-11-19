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

	entry* entries = (entry*) malloc(sizeof(entry) * 125000);

	for (int i = 0; i < 125000; i++) {
		entries[i].key = (uint32_t) rand();
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	cht_build(table, entries, 125000);

	ASSERT_TRUE(125000 >= table->payload_size);
	ASSERT_EQ(125000 * 8 / 32, table->bitmap_size);

	cht_free(table);
}

TEST( CHT, Find) {
	srand(time(NULL));

	cht* table = (cht*) malloc(sizeof(cht));

	entry* entries = (entry*) malloc(sizeof(entry) * 125000);

	for (int i = 0; i < 125000; i++) {
		entries[i].key = (uint32_t) rand();
		if (entries[i].key == 0)
			entries[i].key += 1;
		for (int j = 0; j < 4; j++) {
			entries[i].payload[j] = (uint8_t) (rand() % 0xff);
		}
	}

	// Fill in random data
	cht_build(table, entries, 125000);

	for (int i = 0; i < 125000; i++) {
		entry* entry = cht_find(table, entries[i].key);
		for (int j = 0; j < 4; j++) {
			ASSERT_EQ(entries[i].payload[j], entry->payload[j]);
		}
	}

	cht_free(table);
}
