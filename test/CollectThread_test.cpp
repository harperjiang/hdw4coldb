/*
 * CollectThread_test.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include "../src/join/CollectThread.h"

TEST(CollectThread, Collect) {
	uint input[3227];

	for (int i = 0; i < 3227; i++) {
		if (i % 10)
		input[i] = 2403 + i;
		else
		input[i] = 0;
	}

	NotEqual nz(0);

	uint output[3227];
	ASSERT_EQ(2904, CollectThread::collect(input, output, 3227, &nz));

	for (int i = 0; i < 2904; i++) {
		ASSERT_EQ(i - (i/10+1) + 2403, output[i]);
	}
}
